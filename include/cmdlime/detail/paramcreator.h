#pragma once
#include "param.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail {

template<typename T>
class ParamCreator{
public:
    ParamCreator(ConfigReaderPtr cfgReader,
                 const std::string& varName,
                 const std::string& type,
                 T& paramValue)
        : cfgReader_(cfgReader)
        , paramValue_(paramValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        param_ = std::make_unique<Param<T>>(
                cfgReader_ ? NameFormat::name(cfgReader->format(), varName) : varName,
                cfgReader_ ? NameFormat::shortName(cfgReader->format(), varName) : varName,
                cfgReader_ ? NameFormat::valueName(cfgReader->format(), type) : varName,
                paramValue);
    }

    auto& operator<<(const std::string& info)
    {
        param_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        param_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ShortName& customName)
    {
//        static_assert(FormatCfg<format>::shortNamesEnabled,
//                      "Current command line format doesn't support short names");
        param_->info().resetShortName(customName.value());
        return *this;
    }

    auto& operator<<(const WithoutShortName&)
    {
//        static_assert(FormatCfg<format>::shortNamesEnabled,
//                      "Current command line format doesn't support short names");
        param_->info().resetShortName({});
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        param_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const T&)> validationFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(std::make_unique<Validator<T>>(*param_, paramValue_, std::move(validationFunc)));
        return *this;
    }

    auto& operator()(T defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        param_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator T()
    {
        if (cfgReader_)
            cfgReader_->addParam(std::move(param_));
        return defaultValue_;
    }

private:
    std::unique_ptr<Param<T>> param_;
    T defaultValue_;
    ConfigReaderPtr cfgReader_;
    T& paramValue_;
};

template <typename T>
auto makeParamCreator(ConfigReaderPtr cfgReader,
                      const std::string& varName,
                      const std::string& type,
                      const std::function<T&()>& paramGetter)
{
    return ParamCreator<T>{cfgReader, varName, type, paramGetter()};
}


}