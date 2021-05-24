#pragma once
#include "icommand.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include "streamreader.h"
#include "gsl/assert"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/detail/configaccess.h>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class Command : public ICommand, public ConfigVar{
public:
    enum class Type{
        Normal,
        SubCommand
    };

    Command(const std::string& name,        
            std::function<T*&()> commandGetter,
            Type type)
        : ConfigVar(name, {}, {})
        , commandGetter_(commandGetter)
        , type_(type)
    {
    }

private:
    ConfigVar& info() override
    {
        return *this;
    }

    const ConfigVar& info() const override
    {
        return *this;
    }

    void read(const std::vector<std::string>& commandLine) override
    {
        auto& commandPtr = commandGetter_();
        if (!commandPtr)
            commandPtr = &commandCfg_;
        commandPtr->read(commandLine);
    }

    void setName(const std::string& programName) override
    {        
        programName_ = programName + " " + name();
        detail::ConfigAccess<T>(commandCfg_).setCommandNames(programName_);
    }

    bool isSubCommand() const override
    {
        return type_ == Type::SubCommand;
    }

    std::string usageInfo() const override
    {
        return commandCfg_.usageInfo(programName_);
    }

    std::string usageInfoDetailed() const override
    {        
        return commandCfg_.usageInfoDetailed(programName_);
    }

    void addFlag(std::unique_ptr<IFlag> flag) override
    {                
        detail::ConfigAccess<T>(commandCfg_).addFlag(std::move(flag));
    }

    std::vector<not_null<ICommand*>> commandList() override
    {        
        return detail::ConfigAccess<T>(commandCfg_).commandList();
    }

private:
    std::function<T*&()> commandGetter_;
    Type type_;
    std::string programName_;
    T commandCfg_;
};

template<typename T, typename TConfig>
class CommandCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    CommandCreator(TConfig& cfg,
                   const std::string& varName,
                   std::function<T*&()> commandGetter,
                   typename Command<T>::Type type = Command<T>::Type::Normal)
        : cfg_(cfg)
    {
        Expects(!varName.empty());        
        command_ = std::make_unique<Command<T>>(NameProvider::argName(varName), commandGetter, type);
    }

    CommandCreator<T, TConfig>& operator<<(const std::string& info)
    {
        command_->addDescription(info);
        return *this;
    }

    CommandCreator<T, TConfig>& operator<<(const Name& customName)
    {
        command_->resetName(customName.value());
        return *this;
    }

    operator T*()
    {
        ConfigAccess<TConfig>{cfg_}.addCommand(std::move(command_));
        return nullptr;
    }

private:
    std::unique_ptr<Command<T>> command_;
    TConfig& cfg_;
};

template <typename T, typename TConfig>
CommandCreator<T, TConfig> makeCommandCreator(TConfig& cfg,
                                              const std::string& varName,
                                              std::function<T*&()> commandGetter,
                                              typename Command<T>::Type type = Command<T>::Type::Normal)
{
    return CommandCreator<T, TConfig>{cfg, varName, commandGetter, type};
}


}
