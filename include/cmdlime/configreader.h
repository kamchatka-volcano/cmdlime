#pragma once
#include "errors.h"
#include "usageinfoformat.h"
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/configaccess.h"
#include "detail/flag.h"
#include "detail/gsl/pointers"
#include <iostream>
#include <optional>
#include <map>

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
        , programName_(programName)
        , usageInfoFormat_(usageInfoFormat)
        , errorOutputMode_(errorOutputMode)
    {}

    int exitCode() const
    {
        return exitCode_;
    }

    void registerCommandHelpFlag(gsl::not_null<detail::ICommand*> command)
    {
        auto id = 0;
        if (!commandHelpFlags_.empty())
            id = commandHelpFlags_.rbegin()->first + 1;

        auto& helpFlagStorage = commandHelpFlags_[id];

        using NameProvider = typename detail::Format<detail::ConfigAccess<TConfig>::format()>::nameProvider;
        auto helpFlag = std::make_unique<detail::Flag>(NameProvider::name("help"),
                                                       std::string{},
                                                       [&helpFlagStorage]()->bool&{return helpFlagStorage.value;},
                                                       detail::Flag::Type::Exit);
        helpFlag->addDescription("show usage info and exit");
        helpFlagStorage.usageInfo = command->usageInfoDetailed();
        command->addFlag(std::move(helpFlag));

        for (auto childCommand : command->commandList())
            registerCommandHelpFlag(childCommand);
    }

    bool read(const std::vector<std::string>& cmdLine)
    {
        using NameProvider = typename detail::Format<detail::ConfigAccess<TConfig>::format()>::nameProvider;
        auto helpFlag = std::make_unique<detail::Flag>(NameProvider::name("help"),
                                                       std::string{},
                                                       [this]()->bool&{return help_;},
                                                       detail::Flag::Type::Exit);
        helpFlag->addDescription("show usage info and exit");
        detail::ConfigAccess<TConfig>{cfg_}.addFlag(std::move(helpFlag));        

        if (!cfg_.versionInfo().empty()){
            auto versionFlag = std::make_unique<detail::Flag>(NameProvider::name("version"),
                                                           std::string{},
                                                           [this]()->bool&{return version_;},
                                                           detail::Flag::Type::Exit);
            versionFlag->addDescription("show version info and exit");
            detail::ConfigAccess<TConfig>{cfg_}.addFlag(std::move(versionFlag));
        }


        detail::ConfigAccess<decltype(cfg_)>(cfg_).setCommandNames(programName_);
        for (auto command :  detail::ConfigAccess<TConfig>{cfg_}.commandList())
            registerCommandHelpFlag(command);

        try{
            cfg_.read(cmdLine);
        }
        catch(const CommandError& e){
            if (errorOutputMode_ == ErrorOutputMode::STDERR)
                std::cerr << "Command '" + e.commandName() + "' error: " << e.what() << std::endl;
            else if (errorOutputMode_ == ErrorOutputMode::STDOUT)
                std::cout << "Command '" + e.commandName() + "' error: " << e.what() << "\n";
            std::cout << e.commandUsageInfo() << std::endl;
            return exitOnError(-1);
        }
        catch(const Error& e){
            if (errorOutputMode_ == ErrorOutputMode::STDERR)
                std::cerr << e.what() << std::endl;
            else if (errorOutputMode_ == ErrorOutputMode::STDOUT)
                std::cout << e.what() << "\n";
            std::cout << cfg_.usageInfo(programName_) << std::endl;
            return exitOnError(-1);
        }

        if (help_){
            std::cout << cfg_.usageInfoDetailed(programName_, usageInfoFormat_) << std::endl;
            return exitOnFlag();
        }
        if (version_){
            std::cout << cfg_.versionInfo() << std::endl;
            return exitOnFlag();
        }

        for (const auto& idFlagPair : commandHelpFlags_){
            const auto& flag = idFlagPair.second;
            if (flag.value){
                std::cout << flag.usageInfo << std::endl;
                return exitOnFlag();
            }
        }


        return success();
    }

    bool readCommandLine(int argc, char** argv)
    {
        auto cmdLine = std::vector<std::string>(argv + 1, argv + argc);
        return read(cmdLine);
    }

private:
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
    TConfig& cfg_;
    std::string programName_;
    UsageInfoFormat usageInfoFormat_;
    ErrorOutputMode errorOutputMode_;
    std::map<int, CommandHelpFlag> commandHelpFlags_;
    int exitCode_ = 0;
    bool help_ = false;
    bool version_ = false;
}
;
}
