#pragma once
#include "paramlist.h"
#include "iconfig.h"
#include "formatcfg.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail {

template<typename T, Format format>
class ParamListCreator{
    using NameProvider = typename FormatCfg<format>::nameProvider;

public:
    ParamListCreator(IConfig& cfg,
                     const std::string& varName,
                     const std::string& type,
                     std::vector<T>& paramListValue)
            : cfg_(cfg)
            , paramListValue_(paramListValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        paramList_ = std::make_unique<ParamList<T>>(NameProvider::name(varName),
                NameProvider::shortName(varName),
                NameProvider::valueName(type),
                paramListValue);
    }

    auto& operator<<(const std::string& info)
    {
        paramList_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        paramList_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ShortName& customName)
    {
        static_assert(FormatCfg<format>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        paramList_->info().resetShortName(customName.value());
        return *this;
    }

    auto& operator<<(const WithoutShortName&)
    {
        static_assert(FormatCfg<format>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        paramList_->info().resetShortName({});
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        paramList_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const std::vector<T>&)> validationFunc)
    {
        cfg_.addValidator(std::make_unique<Validator<std::vector<T>>>(*paramList_, paramListValue_, std::move(validationFunc)));
        return *this;
    }


    auto& operator()(std::vector<T> defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        paramList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator std::vector<T>()
    {
        cfg_.addParamList(std::move(paramList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ParamList<T>> paramList_;
    std::vector<T> defaultValue_;
    IConfig& cfg_;
    std::vector<T>& paramListValue_;
};

template <typename T, typename TConfig>
auto makeParamListCreator(TConfig& cfg,
                          const std::string& varName,
                          const std::string& type,
                          const std::function<std::vector<T>&()>& paramListGetter)
{
return ParamListCreator<T, TConfig::format()>{cfg, varName, type, paramListGetter()};
}


}