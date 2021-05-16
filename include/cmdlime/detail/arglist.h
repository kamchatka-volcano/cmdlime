#pragma once
#include "iarglist.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include "errors.h"
#include "customnames.h"
#include "gsl/assert"
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class ArgList : public IArgList, public ConfigVar{
public:
    ArgList(const std::string& name,
            const std::string& type,
            std::function<std::vector<T>&()> argListGetter)
        : ConfigVar(name, {}, type)
        , argListGetter_(argListGetter)
    {
    }

    void setDefaultValue(const std::vector<T>& value)
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

    bool read(const std::string& data) override
    {
        if (!isDefaultValueOverwritten_){
            argListGetter_().clear();
            isDefaultValueOverwritten_ = true;
        }
        auto stream = std::stringstream{data};        
        argListGetter_().emplace_back();
        stream >> argListGetter_().back();
        if (stream.bad() || stream.fail() || !stream.eof())
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
        stream << "{";
        auto firstVal = true;
        for (auto& val : defaultValue_.value()){
            if (firstVal)
                stream << val;
            else
                stream << ", " << val;
            firstVal = false;
        }
        stream << "}";
        return stream.str();
    }

private:        
    std::function<std::vector<T>&()> argListGetter_;
    bool hasValue_ = false;
    std::optional<std::vector<T>> defaultValue_;
    bool isDefaultValueOverwritten_ = false;
};

template <>
inline bool ArgList<std::string>::read(const std::string& data)
{
    argListGetter_().push_back(data);
    hasValue_ = true;
    return true;
}

template<typename T, typename TConfig>
class ArgListCreator{
    using NameProvider = typename Format<TConfig::format>::nameProvider;

public:
    ArgListCreator(TConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::function<std::vector<T>&()> argListGetter)
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        argList_ = std::make_unique<ArgList<T>>(NameProvider::argName(varName),
                                                NameProvider::valueName(type), argListGetter);
    }

    ArgListCreator<T, TConfig>& operator<<(const std::string& info)
    {
        argList_->addDescription(info);
        return *this;
    }

    ArgListCreator<T, TConfig>& operator<<(const Name& customName)
    {
        argList_->resetName(customName.value());
        return *this;
    }

    ArgListCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        argList_->resetValueName(valueName.value());
        return *this;
    }

    ArgListCreator<T, TConfig>& operator()(const std::vector<T>& defaultValue = {})
    {
        defaultValue_ = defaultValue;
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


}
