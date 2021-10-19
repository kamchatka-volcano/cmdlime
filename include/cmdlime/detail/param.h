#pragma once
#include "iparam.h"
#include "configaccess.h"
#include "optioninfo.h"
#include "format.h"
#include "streamreader.h"
#include <gsl/gsl>
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <sstream>
#include <optional>
#include <memory>
#include <functional>

namespace cmdlime::detail{

template<typename T>
class Param : public IParam{
public:
    Param(std::string name,
          std::string shortName,
          std::string type,
          std::function<T&()> paramGetter)
        : info_(std::move(name), std::move(shortName), std::move(type))
        , paramGetter_(std::move(paramGetter))
    {       
    }

    void setDefaultValue(const T& value)
    {
        hasValue_ = true;
        defaultValue_ = value;
    }

    OptionInfo& info() override
    {
        return info_;
    }

    const OptionInfo& info() const override
    {
        return info_;
    }

private:
    bool read(const std::string& data) override
    {
        auto stream = std::stringstream{data};
        if (!readFromStream(stream, paramGetter_()))
            return false;
        hasValue_ = true;
        return true;
    }

    bool hasValue() const override
    {
        return hasValue_;
    }

    bool isOptional() const override
    {
        return defaultValue_.has_value();
    }

    std::string defaultValue() const override
    {
        if (!defaultValue_.has_value())
            return {};
        auto stream = std::stringstream{};
        stream << defaultValue_.value();
        return stream.str();
    }

private:
    OptionInfo info_;
    std::function<T&()> paramGetter_;
    std::optional<T> defaultValue_;
    bool hasValue_ = false;
};

template <>
inline bool Param<std::string>::read(const std::string& data)
{    
    paramGetter_() = data;
    hasValue_ = true;
    return true;
}

template<typename T, typename TConfig>
class ParamCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    ParamCreator(TConfig& cfg,
                 const std::string& varName,
                 const std::string& type,
                 std::function<T&()> paramGetter)
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        param_ = std::make_unique<Param<T>>(NameProvider::name(varName),
                                            NameProvider::shortName(varName),
                                            NameProvider::valueName(type),
                                            std::move(paramGetter));
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
                                          std::function<T&()> paramGetter)
{
    return ParamCreator<T, TConfig>{cfg, varName, type, std::move(paramGetter)};
}

}
