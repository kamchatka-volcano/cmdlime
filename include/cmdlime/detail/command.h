#pragma once
#include "icommand.h"
#include "optioninfo.h"
#include "flag.h"
#include "iconfigreader.h"
#include "options.h"
#include "nameformat.h"
#include "initializedoptional.h"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/usageinfoformat.h>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{
using namespace gsl;

template <typename TConfig>
class Command : public ICommand{
public:
    enum class Type{
        Normal,
        SubCommand
    };

    Command(const std::string& name,
            InitializedOptional<TConfig>& commandCfg,
            ConfigReaderPtr cfgReader,
            Type type)
        : info_(name, {}, {})
        , type_(type)
        , cfg_(commandCfg)
        , cfgReader_{cfgReader}
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

    OptionType type() const override
    {
        return isSubCommand() ? OptionType::Subcommand : OptionType::Command;
    }

private:
    ConfigReadResult read(const std::vector<std::string>& commandLine) override
    {
        cfg_.emplace();
        if (!cfgReader_)
            return ConfigReadResult::Completed;

        cfgReader_->setCommandName(commandName_);
        cfgReader_->setUsageInfoFormat(commandUsageInfoFormat_);
        if (helpFlag_){
            cfgReader_->addFlag(std::move(helpFlag_));
            for (auto& command : cfgReader_->options().commands())
                command->enableHelpFlag();
        }

        return cfgReader_->read(commandLine);
    }

    ConfigReaderPtr configReader() const override
    {
        return cfgReader_;
    }

    void enableHelpFlag() override
    {
        helpFlag_ = std::make_unique<detail::Flag>(NameFormat::name(cfgReader_->format(), "help"),
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

    bool hasValue() const override
    {
        return cfg_;
    }

    std::string usageInfo() const override
    {
        if (!cfgReader_)
            return {};
        return cfgReader_->usageInfo();
    }

    std::string usageInfoDetailed() const override
    {
        if (!cfgReader_)
            return {};
        return cfgReader_->usageInfoDetailed();
    }

    void setUsageInfoFormat(const UsageInfoFormat& format) override
    {
        commandUsageInfoFormat_ = format;
    }

    void setCommandName(const std::string& parentCommandName) override
    {
        commandName_ = parentCommandName.empty() ? info_.name() :
                       parentCommandName + " " + info_.name();
    }

    void validate() const override
    {
        if (cfgReader_ && cfg_)
            cfgReader_->validate(info_.name());
    }


private:
    OptionInfo info_;
    Type type_;
    UsageInfoFormat commandUsageInfoFormat_;
    InitializedOptional<TConfig>& cfg_;
    ConfigReaderPtr cfgReader_;
    std::string commandName_;
    std::unique_ptr<IFlag> helpFlag_;
    bool helpFlagValue_ = false;
};

}
