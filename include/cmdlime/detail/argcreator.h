#pragma once
#include "arg.h"
#include "iconfig.h"
#include "formatcfg.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail{

template<typename T, Format format>
class ArgCreator{
    using NameProvider = typename FormatCfg<format>::nameProvider;
public:
    ArgCreator(IConfig& cfg,
               const std::string& varName,
               const std::string& type,
               T& argValue)
            : cfg_(cfg)
            , argValue_(argValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        arg_ = std::make_unique<Arg<T>>(NameProvider::fullName(varName),
                NameProvider::valueName(type),
                argValue);
    }

    auto& operator<<(const std::string& info)
    {
        arg_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        arg_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        arg_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const T&)> validationFunc)
    {
        cfg_.addValidator(std::make_unique<Validator<T>>(*arg_, argValue_, std::move(validationFunc)));
        return *this;
    }

    operator T()
    {
        cfg_.addArg(std::move(arg_));
        return T{};
    }

private:
    std::unique_ptr<Arg<T>> arg_;
    IConfig& cfg_;
    T& argValue_;
};

template <typename T, typename TConfig>
auto makeArgCreator(TConfig& cfg, const std::string& varName,
                                  const std::string& type,
                                  const std::function<T&()>& argGetter)
{
    return ArgCreator<T, TConfig::format()>{cfg, varName, type, argGetter()};
}


}