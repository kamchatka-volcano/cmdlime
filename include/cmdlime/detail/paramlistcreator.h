#pragma once
#include "paramlist.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail {

template<typename T>
class ParamListCreator{
public:
    ParamListCreator(ConfigReaderPtr cfgReader,
                     const std::string& varName,
                     const std::string& type,
                     std::vector<T>& paramListValue)
            : cfgReader_(cfgReader)
            , paramListValue_(paramListValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        paramList_ = std::make_unique<ParamList<T>>(
                cfgReader_ ? NameFormat::name(cfgReader_->format(), varName) : varName,
                cfgReader_ ? NameFormat::shortName(cfgReader_->format(), varName) : varName,
                cfgReader_ ? NameFormat::valueName(cfgReader_->format(), type) : type,
                paramListValue);
    }

    auto& operator<<(const std::string& info)
    {
        paramList_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        paramList_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ShortName& customName)
    {
        if (cfgReader_ && cfgReader_->shortNamesEnabled())
            paramList_->info().resetShortName(customName.value());
        return *this;
    }

    auto& operator<<(const WithoutShortName&)
    {
        if (cfgReader_ && cfgReader_->shortNamesEnabled())
            paramList_->info().resetShortName({});
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        paramList_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const std::vector<T>&)> validationFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(
                    std::make_unique<Validator<std::vector<T>>>(*paramList_, paramListValue_, std::move(validationFunc)));
        return *this;
    }


    auto& operator()(std::vector<T> defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        paramList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator std::vector<T>()
    {
        if (cfgReader_)
            cfgReader_->addParamList(std::move(paramList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ParamList<T>> paramList_;
    std::vector<T> defaultValue_;
    ConfigReaderPtr cfgReader_;
    std::vector<T>& paramListValue_;
};

}