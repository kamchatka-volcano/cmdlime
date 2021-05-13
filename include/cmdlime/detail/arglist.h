#pragma once
#include "iarglist.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include "errors.h"
#include "customnames.h"
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class ArgList : public IArgList, public ConfigVar{
public:
    ArgList(const std::string& originalName,
            const std::string& name,
            const std::string& shortName,
            const std::string& type,
            std::function<std::vector<T>&()> argListGetter)
        : ConfigVar(originalName, name, shortName, type)
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

    void read(const std::string& data) override
    {
        if (!isDefaultValueOverwritten_){
            argListGetter_().clear();
            isDefaultValueOverwritten_ = true;
        }
        auto stream = std::stringstream{data};
        stream.exceptions(std::stringstream::failbit | std::stringstream::badbit);
        argListGetter_().emplace_back();
        try{
            stream >> argListGetter_().back();
        }
        catch(const std::exception&){
            throw ParsingError{"Couldn't set argument list '" + name() + "' value from '" + data + "'"};
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
inline void ArgList<std::string>::read(const std::string& data)
{
    argListGetter_().push_back(data);
    hasValue_ = true;
}

template<typename T, typename TConfig>
class ArgListCreator{
    using NameProvider = typename Format<TConfig::format>::nameProvider;

public:
    ArgListCreator(TConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::function<std::vector<T>&()> argListGetter)
        : argList_(std::make_unique<ArgList<T>>(varName,
                                                NameProvider::name(varName),
                                                NameProvider::shortName(varName),
                                                NameProvider::valueName(type), argListGetter))
        , cfg_(cfg)
    {}

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

    ArgListCreator<T, TConfig>& operator<<(const ShortName& customName)
    {
        static_assert(Format<TConfig::format>::shortNamesEnabled, "Current command line format doesn't support short names");
        argList_->resetShortName(customName.value());
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
