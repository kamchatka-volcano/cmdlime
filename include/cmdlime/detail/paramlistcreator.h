#pragma once
#include "paramlist.h"
#include "configaccess.h"
#include "format.h"

namespace cmdlime::detail {

template<typename T, typename TConfig>
class ParamListCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;

public:
    ParamListCreator(TConfig& cfg,
                     const std::string& varName,
                     const std::string& type,
                     std::vector<T>& paramListValue)
            : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        paramList_ = std::make_unique<ParamList<T>>(NameProvider::name(varName),
                NameProvider::shortName(varName),
                NameProvider::valueName(type),
                paramListValue);
    }

    ParamListCreator<T, TConfig>& operator<<(const std::string& info)
    {
        paramList_->info().addDescription(info);
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const Name& customName)
    {
        paramList_->info().resetName(customName.value());
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const ShortName& customName)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        paramList_->info().resetShortName(customName.value());
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const WithoutShortName&)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        paramList_->info().resetShortName({});
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        paramList_->info().resetValueName(valueName.value());
        return *this;
    }

    ParamListCreator<T, TConfig>& operator()(std::vector<T> defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        paramList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator std::vector<T>()
    {
        ConfigAccess<TConfig>{cfg_}.addParamList(std::move(paramList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ParamList<T>> paramList_;
    std::vector<T> defaultValue_;
    TConfig& cfg_;
};

template <typename T, typename TConfig>
ParamListCreator<T, TConfig> makeParamListCreator(TConfig& cfg,
                                                  const std::string& varName,
                                                  const std::string& type,
                                                  std::function<std::vector<T>&()> paramListGetter)
{
return ParamListCreator<T, TConfig>{cfg, varName, type, paramListGetter()};
}


}