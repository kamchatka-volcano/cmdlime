#pragma once
#include "param.h"
#include "configaccess.h"
#include "format.h"

namespace cmdlime::detail {

template<typename T, typename TConfig>
class ParamCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    ParamCreator(TConfig& cfg,
                 const std::string& varName,
                 const std::string& type,
                 T& paramValue)
            : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        param_ = std::make_unique<Param<T>>(NameProvider::name(varName),
                NameProvider::shortName(varName),
                NameProvider::valueName(type),
                paramValue);
    }

    ParamCreator<T, TConfig>& operator<<(const std::string& info)
    {
        param_->info().addDescription(info);
        return *this;
    }

    ParamCreator<T, TConfig>& operator<<(const Name& customName)
    {
        param_->info().resetName(customName.value());
        return *this;
    }

    ParamCreator<T, TConfig>& operator<<(const ShortName& customName)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        param_->info().resetShortName(customName.value());
        return *this;
    }

    ParamCreator<T, TConfig>& operator<<(const WithoutShortName&)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        param_->info().resetShortName({});
        return *this;
    }

    ParamCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        param_->info().resetValueName(valueName.value());
        return *this;
    }

    ParamCreator<T, TConfig>& operator()(T defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        param_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator T()
    {
        ConfigAccess<TConfig>{cfg_}.addParam(std::move(param_));
        return defaultValue_;
    }

private:
    std::unique_ptr<Param<T>> param_;
    T defaultValue_;
    TConfig& cfg_;
};

template <typename T, typename TConfig>
ParamCreator<T, TConfig> makeParamCreator(TConfig& cfg,
                                          const std::string& varName,
                                          const std::string& type,
                                          const std::function<T&()>& paramGetter)
{
    return ParamCreator<T, TConfig>{cfg, varName, type, paramGetter()};
}


}