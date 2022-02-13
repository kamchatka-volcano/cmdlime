#pragma once
#include "arglist.h"
#include "iconfig.h"
#include "formatcfg.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail{

template<typename T, Format format>
class ArgListCreator{
    using NameProvider = typename FormatCfg<format>::nameProvider;

public:
    ArgListCreator(IConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::vector<T>& argListValue)
            : cfg_(cfg)
            , argListValue_(argListValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        argList_ = std::make_unique<ArgList<T>>(NameProvider::fullName(varName),
                                                NameProvider::valueName(type),
                                                argListValue);
    }

    auto& operator<<(const std::string& info)
    {
        argList_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        argList_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        argList_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator <<(std::function<void(const std::vector<T>&)> validationFunc)
    {
        cfg_.addValidator(std::make_unique<Validator<std::vector<T>>>(*argList_, argListValue_, std::move(validationFunc)));
        return *this;
    }

    auto& operator()(std::vector<T> defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        argList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator std::vector<T>()
    {
        cfg_.setArgList(std::move(argList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ArgList<T>> argList_;
    std::vector<T> defaultValue_;
    IConfig& cfg_;
    std::vector<T>& argListValue_;
};

template <typename T, typename TConfig>
auto makeArgListCreator(TConfig& cfg,
                        const std::string& varName,
                        const std::string& type,
                        const std::function<std::vector<T>&()>& argListGetter)
{
    return ArgListCreator<T, TConfig::format()>{cfg, varName, type, argListGetter()};
}

}