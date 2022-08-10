#pragma once
#include "icommand.h"
#include "optioninfo.h"
#include "flag.h"
#include "icommandlinereader.h"
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
            CommandLineReaderPtr reader,
            Type type)
        : info_(name, {}, {})
        , type_(type)
        , cfg_(commandCfg)
        , reader_{reader}
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
    CommandLineReadResult read(const std::vector<std::string>& commandLine) override
    {
        cfg_.emplace();
        if (!reader_)
            return CommandLineReadResult::Completed;

        reader_->setCommandName(commandName_);
        reader_->setUsageInfoFormat(commandUsageInfoFormat_);
        if (helpFlag_){
            reader_->addFlag(std::move(helpFlag_));
            for (auto& command : reader_->options().commands())
                command->enableHelpFlag();
        }

        return reader_->read(commandLine);
    }

    CommandLineReaderPtr configReader() const override
    {
        return reader_;
    }

    void enableHelpFlag() override
    {
        helpFlag_ = std::make_unique<detail::Flag>(NameFormat::name(reader_->format(), "help"),
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
        if (!reader_)
            return {};
        return reader_->usageInfo();
    }

    std::string usageInfoDetailed() const override
    {
        if (!reader_)
            return {};
        return reader_->usageInfoDetailed();
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
        if (reader_ && cfg_)
            reader_->validate(info_.name());
    }


private:
    OptionInfo info_;
    Type type_;
    UsageInfoFormat commandUsageInfoFormat_;
    InitializedOptional<TConfig>& cfg_;
    CommandLineReaderPtr reader_;
    std::string commandName_;
    std::unique_ptr<IFlag> helpFlag_;
    bool helpFlagValue_ = false;
};

}
