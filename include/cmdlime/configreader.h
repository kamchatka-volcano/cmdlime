#pragma once
#include "config.h"
#include "errors.h"
#include "usageinfoformat.h"
#include "detail/flag.h"
#include "detail/configmacro.h"
#include "detail/nameformat.h"
#include <iostream>
#include <optional>
#include <map>
#include <utility>
#include <functional>


namespace cmdlime{

template<Format formatType>
class ConfigReader : public detail::IConfigReader{
public:
    ConfigReader(const std::string& programName = {},
                 const UsageInfoFormat& usageInfoFormat = {})
    {
        setCommandName(programName);
        setUsageInfoFormat(usageInfoFormat);
    }

    Format format() const override
    {
        return formatType;
    }

    template<typename TCfg>
    TCfg read(int argc, char** argv)
    {
        return read<TCfg>(std::vector<std::string>{argv + 1, argv + argc});
    }

    template<typename TCfg>
    TCfg read(const std::vector<std::string>& cmdLine)
    {
        auto cfg = makeCfg<TCfg>();
        if (read(cmdLine) != detail::ConfigReadResult::StoppedOnExitFlag)
            validate({});
        return cfg;
    }

    template<typename TCfg>
    int exec(int argc, char** argv, std::function<int(const TCfg&)> func)
    {
         auto cmdLine = std::vector<std::string>(argv + 1, argv + argc);
         return exec<TCfg>(cmdLine, func);
    }

    template<typename TCfg>
    int exec(const std::vector<std::string>& cmdLine, std::function<int(const TCfg&)> func)
    {
        auto cfg = makeCfg<TCfg>();
        addDefaultFlags();

        try {
            if (read(cmdLine) != detail::ConfigReadResult::StoppedOnExitFlag)
                validate({});
        }
        catch(const CommandError& e){
            errorOutput_.get() << "Command '" + e.commandName() + "' error: " << e.what() << "\n";
            output_.get() << e.commandUsageInfo() << std::endl;
            return -1;
        }
        catch(const Error& e){
            errorOutput_.get() << e.what() << "\n";
            output_.get() << usageInfo() << std::endl;
            return -1;
        }

        if (processDefaultFlags())
            return 0;

        return func(cfg);
    }

    const std::string& versionInfo() const override
    {
        return versionInfo_;
    }

    template<typename TCfg>
    std::string usageInfo()
    {
        makeCfg<TCfg>();
        return usageInfo();
    }

    template<typename TCfg>
    std::string usageInfoDetailed()
    {
        makeCfg<TCfg>();
        return usageInfoDetailed();
    }

    void setVersionInfo(const std::string& info)
    {
        versionInfo_ = info;
    }

    void setUsageInfo(const std::string& info)
    {
        customUsageInfo_ = info;
    }

    void setUsageInfoDetailed(const std::string& info)
    {
        customUsageInfoDetailed_ = info;
    }

    void setProgramName(const std::string& name)
    {
        setCommandName(name);
    }

    void setUsageInfoFormat(const UsageInfoFormat& format) override
    {
        usageInfoFormat_ = format;
        for (auto& command : options_.commands())
            command->setUsageInfoFormat(format);
    }

    void setOutputStream(std::ostream& outStream)
    {
        output_ = outStream;
    }

    void setErrorOutputStream(std::ostream& outStream)
    {
        errorOutput_ = outStream;
    }

    void addParam(std::unique_ptr<detail::IParam> param) override
    {
        options_.addParam(std::move(param));
    }

    void addParamList(std::unique_ptr<detail::IParamList> paramList) override
    {
        options_.addParamList(std::move(paramList));
    }

    void addFlag(std::unique_ptr<detail::IFlag> flag) override
    {
        options_.addFlag(std::move(flag));
    }

    void addArg(std::unique_ptr<detail::IArg> arg) override
    {
        options_.addArg(std::move(arg));
    }

    void setArgList(std::unique_ptr<detail::IArgList> argList) override
    {
        if (argListSet_) {
            configError_ = "BaseConfig can have only one arguments list";
            return;
        }
        options_.setArgList(std::move(argList));
        argListSet_ = true;
    }

    void addCommand(std::unique_ptr<detail::ICommand> command) override
    {
        options_.addCommand(std::move(command));
    }

    const detail::Options& options() const override
    {
        return options_;
    }

    void setCommandName(const std::string& name) override
    {
        commandName_ = name;
        for (auto& command : options_.commands())
            command->setCommandName(name);
    }

    void addValidator(std::unique_ptr<detail::IValidator> validator) override
    {
        validators_.emplace_back(std::move(validator));
    }

    void validate(const std::string& commandName) const override
    {
        auto commandIsSet = false;
        for (auto& command : options_.commands()) {
            command->validate();
            if (command->hasValue() && !command->isSubCommand())
                commandIsSet = true;
        }
        for (auto& validator : validators_) {
            if (commandIsSet && validator->optionType() != detail::OptionType::Command)
                continue;
            validator->validate(commandName);
        }

    }

private:
    detail::ConfigReadResult read(const std::vector<std::string>& cmdLine) override
    {
        if (!configError_.empty())
            throw ConfigError{configError_};
        using ParserType = typename detail::FormatCfg<formatType>::parser;
        auto parser = ParserType{options_};
        return parser.parse(cmdLine);
    }

    detail::ConfigReaderPtr makeNestedReader(const std::string& name) override
    {
        nestedReaders_.emplace(name, std::make_unique<ConfigReader<formatType>>());
        return nestedReaders_[name]->makePtr();
    }

    void swapContents(detail::ConfigReaderPtr otherReader) override
    {
        auto& reader = static_cast<ConfigReader<formatType>&>(*otherReader);
        std::swap(versionInfo_, reader.versionInfo_);
        std::swap(customUsageInfo_, reader.customUsageInfo_);
        std::swap(customUsageInfoDetailed_, reader.customUsageInfoDetailed_);
        std::swap(configError_, reader.configError_);
        std::swap(options_, reader.options_);
        std::swap(commandName_, reader.commandName_);
        std::swap(usageInfoFormat_, reader.usageInfoFormat_);
        std::swap(validators_, reader.validators_);
        std::swap(argListSet_, reader.argListSet_);
        std::swap(errorOutput_, reader.errorOutput_);
        std::swap(output_, reader.output_);
        std::swap(help_, reader.help_);
        std::swap(version_, reader.version_);
    }

    std::string usageInfo() const override
    {
        if (!customUsageInfo_.empty())
            return customUsageInfo_;

        return detail::UsageInfoCreator<formatType>{commandName_, usageInfoFormat_, options_}.create();
    }

    std::string usageInfoDetailed() const override
    {
        if (!customUsageInfoDetailed_.empty())
            return customUsageInfoDetailed_;

        return detail::UsageInfoCreator<formatType>{commandName_, usageInfoFormat_, options_}.createDetailed();
    }

    void clear()
    {
        configError_.clear();
        options_= detail::Options{};
        validators_.clear();
        argListSet_ = false;
        nestedReaders_.clear();
    }

    template<typename TCfg>
    TCfg makeCfg()
    {
        clear();
        auto cfg = TCfg{makePtr()};
        setCommandName(commandName_);
        setUsageInfoFormat(usageInfoFormat_);
        return cfg;
    }

    void addDefaultFlags()
    {
        auto helpFlag = std::make_unique<detail::Flag>(detail::NameFormat::name(format(), "help"),
                                                       std::string{},
                                                       help_,
                                                       detail::Flag::Type::Exit);
        helpFlag->info().addDescription("show usage info and exit");
        addFlag(std::move(helpFlag));

        if (!versionInfo().empty()){
            auto versionFlag = std::make_unique<detail::Flag>(detail::NameFormat::name(format(), "version"),
                                                           std::string{},
                                                           version_,
                                                           detail::Flag::Type::Exit);
            versionFlag->info().addDescription("show version info and exit");
            addFlag(std::move(versionFlag));
        }

        for (auto& command : options_.commands())
            command->enableHelpFlag();
    }

    bool processDefaultFlags()
    {
        if (help_){
            output_.get() << usageInfoDetailed() << std::endl;
            return true;
        }
        if (version_){
            output_.get() << versionInfo() << std::endl;
            return true;
        }

        for (auto& command : options_.commands())
            if (checkCommandHelpFlag(*command))
                return true;

        return false;
    }

    bool checkCommandHelpFlag(detail::ICommand& command)
    {
        if (command.isHelpFlagSet()){
            output_.get() << command.usageInfoDetailed() << std::endl;
            return true;
        }

        if (command.hasValue())
            for (auto& childCommand : command.configReader()->options().commands())
                if (checkCommandHelpFlag(*childCommand))
                    return true;

        return false;
    }

private:
    std::string versionInfo_;
    std::string customUsageInfo_;
    std::string customUsageInfoDetailed_;
    std::string configError_;
    detail::Options options_;
    std::string commandName_;
    UsageInfoFormat usageInfoFormat_;
    std::vector<std::unique_ptr<detail::IValidator>> validators_;
    bool argListSet_ = false;

    std::reference_wrapper<std::ostream> errorOutput_ = std::cerr;
    std::reference_wrapper<std::ostream> output_ = std::cout;
    bool help_ = false;
    bool version_ = false;

    std::map<std::string, std::unique_ptr<ConfigReader<formatType>>> nestedReaders_;
};

using GNUConfigReader = ConfigReader<Format::GNU>;
using X11ConfigReader = ConfigReader<Format::X11>;
using POSIXConfigReader = ConfigReader<Format::POSIX>;
using SimpleConfigReader = ConfigReader<Format::POSIX>;

}
