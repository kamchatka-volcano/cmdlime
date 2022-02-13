#pragma once
#include "param.h"
#include "iconfig.h"
#include "formatcfg.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail {

template<typename T, Format format>
class ParamCreator{
    using NameProvider = typename FormatCfg<format>::nameProvider;
public:
    ParamCreator(IConfig& cfg,
                 const std::string& varName,
                 const std::string& type,
                 T& paramValue)
        : cfg_(cfg)
        , paramValue_(paramValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        param_ = std::make_unique<Param<T>>(NameProvider::name(varName),
                NameProvider::shortName(varName),
                NameProvider::valueName(type),
                paramValue);
    }

    auto& operator<<(const std::string& info)
    {
        param_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        param_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ShortName& customName)
    {
        static_assert(FormatCfg<format>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        param_->info().resetShortName(customName.value());
        return *this;
    }

    auto& operator<<(const WithoutShortName&)
    {
        static_assert(FormatCfg<format>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        param_->info().resetShortName({});
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        param_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const T&)> validationFunc)
    {
        cfg_.addValidator(std::make_unique<Validator<T>>(*param_, paramValue_, std::move(validationFunc)));
        return *this;
    }

    auto& operator()(T defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        param_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator T()
    {
        cfg_.addParam(std::move(param_));
        return defaultValue_;
    }

private:
    std::unique_ptr<Param<T>> param_;
    T defaultValue_;
    IConfig& cfg_;
    T& paramValue_;
};

template <typename T, typename TConfig>
auto makeParamCreator(TConfig& cfg,
                      const std::string& varName,
                      const std::string& type,
                      const std::function<T&()>& paramGetter)
{
    return ParamCreator<T, TConfig::format()>{cfg, varName, type, paramGetter()};
}


}