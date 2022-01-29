#pragma once
#include "arglist.h"
#include "configaccess.h"
#include "format.h"

namespace cmdlime::detail{

template<typename T, typename TConfig>
class ArgListCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;

public:
    ArgListCreator(TConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::vector<T>& argListValue)
            : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        argList_ = std::make_unique<ArgList<T>>(NameProvider::fullName(varName),
                                                NameProvider::valueName(type),
                                                argListValue);
    }

    ArgListCreator<T, TConfig>& operator<<(const std::string& info)
    {
        argList_->info().addDescription(info);
        return *this;
    }

    ArgListCreator<T, TConfig>& operator<<(const Name& customName)
    {
        argList_->info().resetName(customName.value());
        return *this;
    }

    ArgListCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        argList_->info().resetValueName(valueName.value());
        return *this;
    }

    ArgListCreator<T, TConfig>& operator()(std::vector<T> defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        argList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator std::vector<T>()
    {
        ConfigAccess<TConfig>{cfg_}.setArgList(std::move(argList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ArgList<T>> argList_;
    std::vector<T> defaultValue_;
    TConfig& cfg_;
};

template <typename T, typename TConfig>
ArgListCreator<T, TConfig> makeArgListCreator(TConfig& cfg,
                                              const std::string& varName,
                                              const std::string& type,
                                              std::function<std::vector<T>&()> argListGetter)
{
    return ArgListCreator<T, TConfig>{cfg, varName, type, argListGetter()};
}

}