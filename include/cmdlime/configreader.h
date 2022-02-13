#pragma once
#include "errors.h"
#include "usageinfoformat.h"
#include "baseconfig.h"
#include "detail/configmacro.h"
#include "detail/flag.h"
#include <iostream>
#include <ostream>
#include <optional>
#include <map>
#include <utility>
#include <functional>


namespace cmdlime{

enum class ErrorOutputMode{
    STDOUT,
    STDERR
};

template<typename TConfig>
class ConfigReader{
    struct CommandHelpFlag{
        bool value = false;
        std::string usageInfo;
    };

public:
    ConfigReader(TConfig& cfg,
                 const std::string& programName,
                 const UsageInfoFormat& usageInfoFormat = {},
                 ErrorOutputMode errorOutputMode = ErrorOutputMode::STDERR)
        : cfg_(cfg)
        , errorOutput_(errorOutputMode == ErrorOutputMode::STDERR ? std::cerr : std::cout)
    {
        cfg_.setCommandName(programName);
        cfg_.setUsageInfoFormat(usageInfoFormat);
    }

    int exitCode() const
    {
        return exitCode_;
    }

    bool read(const std::vector<std::string>& cmdLine)
    {
        addExitFlags();
        if(!processCommandLine(cmdLine))
            return exitOnError(-1);

        if (processFlagsAndExit())
            return exitOnFlag();

        return success();
    }

    bool readCommandLine(int argc, char** argv)
    {
        auto cmdLine = std::vector<std::string>(argv + 1, argv + argc);
        return read(cmdLine);
    }

    void setOutputStream(std::ostream& outStream)
    {
        output_ = outStream;
    }

    void setErrorOutputStream(std::ostream& outStream)
    {
        errorOutput_ = outStream;
    }

private:
    void addExitFlags()
    {
        using NameProvider = typename detail::FormatCfg<TConfig::format()>::nameProvider;
        auto helpFlag = std::make_unique<detail::Flag>(NameProvider::name("help"),
                                                       std::string{},
                                                       help_,
                                                       detail::Flag::Type::Exit);
        helpFlag->info().addDescription("show usage info and exit");
        cfg_.addFlag(std::move(helpFlag));

        if (!cfg_.versionInfo().empty()){
            auto versionFlag = std::make_unique<detail::Flag>(NameProvider::name("version"),
                                                           std::string{},
                                                           version_,
                                                           detail::Flag::Type::Exit);
            versionFlag->info().addDescription("show version info and exit");
            cfg_.addFlag(std::move(versionFlag));
        }

        for (auto& command : cfg_.options().commands())
            command->enableHelpFlag();
    }

    bool processCommandLine(const std::vector<std::string>& cmdLine)
    {
        try{
            cfg_.read(cmdLine);
            cfg_.validate({});
        }
        catch(const CommandError& e){
            errorOutput_.get() << "Command '" + e.commandName() + "' error: " << e.what() << "\n";
            output_.get() << e.commandUsageInfo() << std::endl;
            return false;
        }
        catch(const Error& e){
            errorOutput_.get() << e.what() << "\n";
            output_.get() << cfg_.usageInfo() << std::endl;
            return false;
        }
        return true;
    }

    bool processFlagsAndExit()
    {
        if (help_){
            output_.get() << cfg_.usageInfoDetailed() << std::endl;
            return true;
        }
        if (version_){
            output_.get() << cfg_.versionInfo() << std::endl;
            return true;
        }

        for (auto& command : cfg_.options().commands())
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

        if (command.config())
            for (auto& childCommand : command.config()->options().commands())
                if (checkCommandHelpFlag(*childCommand))
                    return true;

        return false;
    }

    bool exitOnFlag()
    {
        exitCode_ = 0;
        return false;
    }

    bool exitOnError(int errorCode)
    {
        exitCode_ = errorCode;
        return false;
    }

    bool success()
    {
        exitCode_ = 0;
        return true;
    }

private:
    detail::IConfig& cfg_;
    std::reference_wrapper<std::ostream> errorOutput_;
    std::reference_wrapper<std::ostream> output_ = std::cout;
    int exitCode_ = 0;
    bool help_ = false;
    bool version_ = false;
}
;
}
