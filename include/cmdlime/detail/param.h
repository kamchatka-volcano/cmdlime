#pragma once
#include "iparam.h"
#include "configaccess.h"
#include "configvar.h"
#include "format.h"
#include "errors.h"
#include <sstream>
#include <optional>
#include <memory>
#include <functional>

namespace cmdlime::detail{

template<typename T>
std::stringstream& operator >>(std::stringstream& stream, std::optional<T>& val)
{
    auto value = T{};
    stream >> value;
    val = value;
    return stream;
}

template<typename T>
std::stringstream& operator <<(std::stringstream& stream, const std::optional<T>& val)
{
    stream << val.value();
    return stream;
}

template<typename T>
class Param : public IParam, public ConfigVar{
public:
    Param(const std::string& name, const std::string& shortName, const std::string& type, std::function<T&()> paramGetter)
        : ConfigVar(name, shortName, type)
        , paramGetter_(paramGetter)        
    {       
    }

    void setDefaultValue(const T& value)
    {
        hasValue_ = true;
        defaultValue_ = value;
    }

private:
    ConfigVar& info() override
    {
        return *this;
    }

    const ConfigVar& info() const override
    {
        return *this;
    }

    void read(const std::string& data) override
    {
        auto stream = std::stringstream{data};
        stream.exceptions(std::stringstream::failbit | std::stringstream::badbit);
        try{
            stream >> paramGetter_();
        }
        catch(const std::exception&){
            throw ParsingError{"Couldn't set parameter '" + name() + "' value from '" + data + "'"};
        }
        hasValue_ = true;
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
    std::function<T&()> paramGetter_;
    std::optional<T> defaultValue_;
    bool hasValue_ = false;
};

template <>
void Param<std::string>::read(const std::string& data)
{    
    paramGetter_() = data;
    hasValue_ = true;
}

template<typename T, typename TConfig>
class ParamCreator{
    using NameProvider = typename Format<TConfig::format>::nameProvider;
public:
    ParamCreator(TConfig& cfg,
                 const std::string& varName,
                 const std::string& type,
                 std::function<T&()> paramGetter)
        : param_(std::make_unique<Param<T>>(NameProvider::name(varName),
                                            NameProvider::shortName(varName),
                                            type, paramGetter))
        , cfg_(cfg)
    {}

    ParamCreator<T, TConfig>& operator<<(const std::string& info)
    {
        param_->addDescription(info);
        return *this;
    }

    ParamCreator<T, TConfig>& operator()(const T& defaultValue = {})
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

}
