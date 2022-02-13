#pragma once
#include "icommand.h"
#include "optioninfo.h"
#include "flag.h"
#include "iconfig.h"
#include "options.h"
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
            std::optional<TConfig>& commandCfg,
            Type type)
        : info_(name, {}, {})
        , type_(type)
        , makeCfg_([&commandCfg]() -> TConfig&{
            commandCfg.emplace();
            return commandCfg.value();
          })
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
    void read(const std::vector<std::string>& commandLine) override
    {
        if (!cfg_){
            cfg_ = &makeCfg_();
            cfg_->setCommandName(commandName_);
            cfg_->setUsageInfoFormat(commandUsageInfoFormat_);
            if (helpFlag_){
                cfg_->addFlag(std::move(helpFlag_));
                for (auto& command : cfg_->options().commands())
                    command->enableHelpFlag();
            }
        }
        cfg_->read(commandLine);
    }

    void enableHelpFlag() override
    {
        using NameProvider = typename detail::FormatCfg<TConfig::format()>::nameProvider;
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

    IConfig* config() const override
    {
        return cfg_;
    }

    std::string usageInfo() const override
    {
        if (!cfg_)
            return {};
        return cfg_->usageInfo();
    }

    std::string usageInfoDetailed() const override
    {
        if (!cfg_)
            return {};
        return cfg_->usageInfoDetailed();
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
        if (cfg_)
            cfg_->validate(info_.name());
    }


private:
    OptionInfo info_;
    Type type_;
    UsageInfoFormat commandUsageInfoFormat_;
    std::function<IConfig&()> makeCfg_;
    std::string commandName_;
    std::unique_ptr<IFlag> helpFlag_;
    IConfig* cfg_ = nullptr;
    bool helpFlagValue_ = false;
};

}
