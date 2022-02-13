#pragma once
#include "arg.h"
#include "configaccess.h"
#include "format.h"

namespace cmdlime::detail{

template<typename T, typename TConfig>
class ArgCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    ArgCreator(TConfig& cfg,
               const std::string& varName,
               const std::string& type,
               T& argValue)
            : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        arg_ = std::make_unique<Arg<T>>(NameProvider::fullName(varName),
                NameProvider::valueName(type),
                argValue);
    }

    ArgCreator<T, TConfig>& operator<<(const std::string& info)
    {
        arg_->info().addDescription(info);
        return *this;
    }

    ArgCreator<T, TConfig>& operator<<(const Name& customName)
    {
        arg_->info().resetName(customName.value());
        return *this;
    }

    ArgCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        arg_->info().resetValueName(valueName.value());
        return *this;
    }

    operator T()
    {
        ConfigAccess<TConfig>{cfg_}.addArg(std::move(arg_));
        return T{};
    }

private:
    std::unique_ptr<Arg<T>> arg_;
    TConfig& cfg_;
};

template <typename T, typename TConfig>
ArgCreator<T, TConfig> makeArgCreator(TConfig& cfg,
                                      const std::string& varName,
                                      const std::string& type,
                                      const std::function<T&()>& argGetter)
{
    return ArgCreator<T, TConfig>{cfg, varName, type, argGetter()};
}


}