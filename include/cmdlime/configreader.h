#pragma once
#include "errors.h"
#include "usageinfoformat.h"
#include "baseconfig.h"
#include "detail/configmacro.h"
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
            command->enableHelpFlag(programName_);
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

        for (auto& command : cfg_.options().commands())
            if (checkCommandHelpFlag(*command))
                return true;

        return false;
    }

    bool checkCommandHelpFlag(detail::ICommand& command)
    {
        if (command.isHelpFlagSet()){
            std::cout << command.usageInfoDetailed() << std::endl;
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
