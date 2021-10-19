#pragma once
#include "icommand.h"
#include "optioninfo.h"
#include "configaccess.h"
#include "config.h"
#include "format.h"
#include "streamreader.h"
#include <gsl/gsl>
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/detail/configaccess.h>
#include <cmdlime/detail/flag.h>
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
            std::function<std::optional<TConfig>&()> commandGetter,
            Type type)
        : info_(name, {}, {})
        , commandGetter_(commandGetter)
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
        auto& commandCfg = commandGetter_();
        if (!commandCfg.has_value()){
            commandCfg.emplace();
            if (helpFlag_){
                detail::ConfigAccess<TConfig>(*commandCfg).addFlag(std::move(helpFlag_));
                detail::ConfigAccess<TConfig>(*commandCfg).addHelpFlagToCommands(programName_);
            }
        }
        commandCfg->read(commandLine);
    }

    void enableHelpFlag(const std::string& programName) override
    {        
        programName_ = programName + " " + info_.name();
        using NameProvider = typename detail::Format<detail::ConfigAccess<TConfig>::format()>::nameProvider;
        helpFlag_ = std::make_unique<detail::Flag>(NameProvider::name("help"),
                                                   std::string{},
                                                   [this]()->bool&{return helpFlagValue_;},
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
        auto& commandCfg = commandGetter_();
        if (!commandCfg.has_value())
            return {};
        return commandCfg->usageInfo(programName_);
    }

    std::string usageInfoDetailed() const override
    {
        auto& commandCfg = commandGetter_();
        if (!commandCfg.has_value())
            return {};
        return commandCfg->usageInfoDetailed(programName_);
    }

    std::vector<not_null<ICommand*>> commandList() override
    {
        auto& commandCfg = commandGetter_();
        if (!commandCfg.has_value())
            return {};
        return detail::ConfigAccess<TConfig>(*commandCfg).commandList();
    }

private:
    OptionInfo info_;
    std::function<std::optional<TConfig>&()> commandGetter_;
    Type type_;
    std::string programName_;
    std::unique_ptr<IFlag> helpFlag_;
    bool helpFlagValue_ = false;
};

template<typename T, typename TConfig>
class CommandCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    CommandCreator(TConfig& cfg,
                   const std::string& varName,
                   std::function<std::optional<T>&()> commandGetter,
                   typename Command<T>::Type type = Command<T>::Type::Normal)
        : cfg_(cfg)
    {
        static_assert (std::is_base_of_v<Config<ConfigAccess<TConfig>::format()>, T>,
                       "Command's type must be a subclass of Config<FormatType> and have the same format as its parent config.");
        Expects(!varName.empty());        
        command_ = std::make_unique<Command<T>>(NameProvider::fullName(varName), commandGetter, type);
    }

    CommandCreator<T, TConfig>& operator<<(const std::string& info)
    {
        command_->info().addDescription(info);
        return *this;
    }

    CommandCreator<T, TConfig>& operator<<(const Name& customName)
    {
        command_->info().resetName(customName.value());
        return *this;
    }

    operator std::optional<T>()
    {
        ConfigAccess<TConfig>{cfg_}.addCommand(std::move(command_));
        return std::optional<T>{};
    }

private:
    std::unique_ptr<Command<T>> command_;
    TConfig& cfg_;
};

template <typename T, typename TConfig>
CommandCreator<T, TConfig> makeCommandCreator(TConfig& cfg,
                                              const std::string& varName,
                                              std::function<std::optional<T>&()> commandGetter,
                                              typename Command<T>::Type type = Command<T>::Type::Normal)
{
    return CommandCreator<T, TConfig>{cfg, varName, commandGetter, type};
}


}
