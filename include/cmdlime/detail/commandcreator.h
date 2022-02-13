#pragma once
#include "command.h"
#include "configaccess.h"
#include "format.h"

namespace cmdlime::detail{

template<typename T, typename TConfig>
class CommandCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    CommandCreator(TConfig& cfg,
                   const std::string& varName,
                   std::optional<T>& commandValue,
                   typename Command<T>::Type type = Command<T>::Type::Normal)
            : cfg_(cfg)
    {
        Expects(!varName.empty());
        command_ = std::make_unique<Command<T>>(NameProvider::fullName(varName), commandValue, type);
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

}