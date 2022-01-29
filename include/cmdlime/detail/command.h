#pragma once
#include "icommand.h"
#include "optioninfo.h"
#include "configaccess.h"
#include "format.h"
#include "streamreader.h"
#include "flag.h"
#include <gsl/gsl>
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename TConfig>
class Command : public ICommand{
public:
    enum class Type{
        Normal,
        SubCommand
    };

    Command(const std::string& name,        
            std::optional<TConfig>& commandValue,
            Type type)
        : info_(name, {}, {})
        , commandValue_(commandValue)
        , type_(type)
    {
    }

    OptionInfo& info() override
    {
        return info_;
    }

    const OptionInfo& info() const override
    {
        return info_;
    }

private:
    void read(const std::vector<std::string>& commandLine) override
    {
        if (!commandValue_){
            commandValue_.emplace();
            if (helpFlag_){
                detail::ConfigAccess<TConfig>(*commandValue_).addFlag(std::move(helpFlag_));
                detail::ConfigAccess<TConfig>(*commandValue_).addHelpFlagToCommands(programName_);
            }
        }
        commandValue_->read(commandLine);
    }

    void enableHelpFlag(const std::string& programName) override
    {        
        programName_ = programName + " " + info_.name();
        using NameProvider = typename detail::Format<detail::ConfigAccess<TConfig>::format()>::nameProvider;
        helpFlag_ = std::make_unique<detail::Flag>(NameProvider::name("help"),
                                                   std::string{},
                                                   helpFlagValue_,
                                                   detail::Flag::Type::Exit);
        helpFlag_->info().addDescription("show usage info and exit");
    }

    bool isHelpFlagSet() const override
    {
        return helpFlagValue_;
    }

    bool isSubCommand() const override
    {
        return type_ == Type::SubCommand;
    }

    std::string usageInfo() const override
    {
        if (!commandValue_)
            return {};
        return commandValue_->usageInfo(programName_);
    }

    std::string usageInfoDetailed() const override
    {
        if (!commandValue_)
            return {};
        return commandValue_->usageInfoDetailed(programName_);
    }

    std::vector<not_null<ICommand*>> commandList() override
    {
        if (!commandValue_)
            return {};
        return detail::ConfigAccess<TConfig>(*commandValue_).commandList();
    }

private:
    OptionInfo info_;
    std::optional<TConfig>& commandValue_;
    Type type_;
    std::string programName_;
    std::unique_ptr<IFlag> helpFlag_;
    bool helpFlagValue_ = false;
};

}
