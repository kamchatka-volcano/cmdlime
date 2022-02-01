#pragma once
#include "icommand.h"
#include "optioninfo.h"
#include "flag.h"
#include "iconfig.h"
#include "options.h"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <gsl/gsl>
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
        , makeCfg_([&commandCfg]{
            commandCfg.emplace();
            return &commandCfg.value();
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

private:
    void read(const std::vector<std::string>& commandLine) override
    {
        if (!cfg_){
            cfg_ = makeCfg_();
            if (helpFlag_){
                cfg_->addFlag(std::move(helpFlag_));
                for (auto& command : cfg_->options().commands())
                    command->enableHelpFlag(programName_);
            }
        }
        cfg_->read(commandLine);
    }

    void enableHelpFlag(const std::string& programName) override
    {        
        programName_ = programName + " " + info_.name();
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
        return cfg_->usageInfo(programName_);
    }

    std::string usageInfoDetailed() const override
    {
        if (!cfg_)
            return {};
        return cfg_->usageInfoDetailed(programName_);
    }

private:
    OptionInfo info_;
    Type type_;
    std::function<not_null<IConfig*>()> makeCfg_;
    std::string programName_;
    std::unique_ptr<IFlag> helpFlag_;
    IConfig* cfg_ = nullptr;
    bool helpFlagValue_ = false;
};

}
