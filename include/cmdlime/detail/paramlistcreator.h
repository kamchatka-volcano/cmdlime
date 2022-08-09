#pragma once
#include "paramlist.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include <sfun/traits.h>
#include <gsl/assert>

namespace cmdlime::detail {
using namespace sfun::traits;

template<typename TParamList>
class ParamListCreator{
    static_assert(is_dynamic_sequence_container_v<TParamList>, "Param list field must be a sequence container");

public:
    ParamListCreator(ConfigReaderPtr cfgReader,
                     const std::string& varName,
                     const std::string& type,
                     TParamList& paramListValue)
            : cfgReader_(cfgReader)
            , paramListValue_(paramListValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        paramList_ = std::make_unique<ParamList<TParamList>>(
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

    auto& operator<<(std::function<void(const TParamList&)> validationFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(
                    std::make_unique<Validator<TParamList>>(*paramList_, paramListValue_, std::move(validationFunc)));
        return *this;
    }


    auto& operator()(TParamList defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        paramList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator TParamList()
    {
        if (cfgReader_)
            cfgReader_->addParamList(std::move(paramList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ParamList<TParamList>> paramList_;
    TParamList defaultValue_;
    ConfigReaderPtr cfgReader_;
    TParamList& paramListValue_;
};

}