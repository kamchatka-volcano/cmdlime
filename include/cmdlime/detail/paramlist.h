#pragma once
#include "iparamlist.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include "string_utils.h"
#include "streamreader.h"
#include "gsl/assert"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class ParamList : public IParamList, public ConfigVar{
public:
    ParamList(const std::string& name,
              const std::string& shortName,
              const std::string& type,
              std::function<std::vector<T>&()> paramListGetter)
        : ConfigVar(name, shortName, type)
        , paramListGetter_(paramListGetter)
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
            paramListGetter_().clear();
            isDefaultValueOverwritten_ = true;
        }

        const auto dataParts = str::split(data, ',');
        for (const auto& part : dataParts){
            auto stream = std::stringstream{part};            
            paramListGetter_().emplace_back();
            if (!readFromStream(stream, paramListGetter_().back()))
                return false;
        }
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
    std::function<std::vector<T>&()> paramListGetter_;
    bool hasValue_ = false;
    std::optional<std::vector<T>> defaultValue_;
    bool isDefaultValueOverwritten_ = false;
};

template <>
inline bool ParamList<std::string>::read(const std::string& data)
{
    const auto dataParts = str::split(data, ',');
    for (const auto& part : dataParts){
        paramListGetter_().push_back(part);
    }
    hasValue_ = true;
    return true;
}

template<typename T, typename TConfig>
class ParamListCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;

public:
    ParamListCreator(TConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::function<std::vector<T>&()> paramListGetter)
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        paramList_ = std::make_unique<ParamList<T>>(NameProvider::name(varName),
                                                    NameProvider::shortName(varName),
                                                    NameProvider::valueName(type), paramListGetter);
    }

    ParamListCreator<T, TConfig>& operator<<(const std::string& info)
    {
        paramList_->addDescription(info);
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const Name& customName)
    {
        paramList_->resetName(customName.value());
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const ShortName& customName)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        paramList_->resetShortName(customName.value());
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const WithoutShortName&)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        paramList_->resetShortName({});
        return *this;
    }

    ParamListCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        paramList_->resetValueName(valueName.value());
        return *this;
    }

    ParamListCreator<T, TConfig>& operator()(const std::vector<T>& defaultValue = {})
    {
        defaultValue_ = defaultValue;
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
    return ParamListCreator<T, TConfig>{cfg, varName, type, paramListGetter};
}

}
