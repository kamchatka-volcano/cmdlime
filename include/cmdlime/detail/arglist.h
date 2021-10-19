#pragma once
#include "iarglist.h"
#include "optioninfo.h"
#include "configaccess.h"
#include "format.h"
#include "streamreader.h"
#include <gsl/gsl>
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class ArgList : public IArgList{
public:
    ArgList(std::string name,
            std::string type,
            std::function<std::vector<T>&()> argListGetter)
        : info_(std::move(name), {}, std::move(type))
        , argListGetter_(std::move(argListGetter))
    {
    }

    void setDefaultValue(const std::vector<T>& value)
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
        if (!isDefaultValueOverwritten_){
            argListGetter_().clear();
            isDefaultValueOverwritten_ = true;
        }
        auto stream = std::stringstream{data};        
        argListGetter_().emplace_back();
        if (!readFromStream(stream, argListGetter_().back()))
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
    OptionInfo info_;
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
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;

public:
    ArgListCreator(TConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::function<std::vector<T>&()> argListGetter)
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        argList_ = std::make_unique<ArgList<T>>(NameProvider::fullName(varName),
                                                NameProvider::valueName(type),
                                                std::move(argListGetter));
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
    return ArgListCreator<T, TConfig>{cfg, varName, type, std::move(argListGetter)};
}

}
