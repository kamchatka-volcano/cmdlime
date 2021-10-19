#pragma once
#include "errors.h"
#include "usageinfoformat.h"
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/configaccess.h"
#include "detail/flag.h"
#include <gsl/gsl>
#include <iostream>
#include <ostream>
#include <optional>
#include <map>
#include <utility>

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
                 std::string programName,
                 const UsageInfoFormat& usageInfoFormat = {},
                 ErrorOutputMode errorOutputMode = ErrorOutputMode::STDERR)
        : cfg_(cfg)
        , programName_(std::move(programName))
        , usageInfoFormat_(usageInfoFormat)
        , errorOutput_(errorOutputMode == ErrorOutputMode::STDERR ? std::cerr : std::cout)
    {}

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

private:
    void addExitFlags()
    {
        using NameProvider = typename detail::Format<detail::ConfigAccess<TConfig>::format()>::nameProvider;
        auto helpFlag = std::make_unique<detail::Flag>(NameProvider::name("help"),
                                                       std::string{},
                                                       [this]()->bool&{return help_;},
                                                       detail::Flag::Type::Exit);
        helpFlag->info().addDescription("show usage info and exit");
        detail::ConfigAccess<TConfig>{cfg_}.addFlag(std::move(helpFlag));

        if (!cfg_.versionInfo().empty()){
            auto versionFlag = std::make_unique<detail::Flag>(NameProvider::name("version"),
                                                           std::string{},
                                                           [this]()->bool&{return version_;},
                                                           detail::Flag::Type::Exit);
            versionFlag->info().addDescription("show version info and exit");
            detail::ConfigAccess<TConfig>{cfg_}.addFlag(std::move(versionFlag));
        }

        detail::ConfigAccess<TConfig>(cfg_).addHelpFlagToCommands(programName_);
    }

    bool processCommandLine(const std::vector<std::string>& cmdLine)
    {
        try{
            cfg_.read(cmdLine);
        }
        catch(const CommandError& e){
            errorOutput_ << "Command '" + e.commandName() + "' error: " << e.what() << "\n";
            std::cout << e.commandUsageInfo() << std::endl;
            return false;
        }
        catch(const Error& e){
            errorOutput_ << e.what() << "\n";
            std::cout << cfg_.usageInfo(programName_) << std::endl;
            return false;
        }
        return true;
    }

    bool processFlagsAndExit()
    {
        if (help_){
            std::cout << cfg_.usageInfoDetailed(programName_, usageInfoFormat_) << std::endl;
            return true;
        }
        if (version_){
            std::cout << cfg_.versionInfo() << std::endl;
            return true;
        }

        for (auto command :  detail::ConfigAccess<TConfig>{cfg_}.commandList())
            if (checkCommandHelpFlag(command))
                return true;

        return false;
    }

    bool checkCommandHelpFlag(gsl::not_null<detail::ICommand*> command)
    {
        if (command->isHelpFlagSet()){
            std::cout << command->usageInfoDetailed() << std::endl;
            return true;
        }

        for (auto childCommand : command->commandList())
            if (checkCommandHelpFlag(childCommand))
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
    TConfig& cfg_;
    std::string programName_;
    UsageInfoFormat usageInfoFormat_;
    std::map<int, CommandHelpFlag> commandHelpFlags_;
    std::ostream& errorOutput_;
    int exitCode_ = 0;
    bool help_ = false;
    bool version_ = false;
}
;
}
