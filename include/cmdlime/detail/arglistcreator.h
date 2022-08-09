#pragma once
#include "arglist.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include <sfun/traits.h>
#include <gsl/assert>


namespace cmdlime::detail{

template<typename TArgList>
class ArgListCreator{
    static_assert(is_dynamic_sequence_container_v<TArgList>, "Argument list field must be a sequence container");

public:
    ArgListCreator(ConfigReaderPtr cfgReader,
                   const std::string& varName,
                   const std::string& type,
                   TArgList& argListValue)
            : cfgReader_(cfgReader)
            , argListValue_(argListValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        argList_ = std::make_unique<ArgList<TArgList>>(
                cfgReader_ ? NameFormat::fullName(cfgReader_->format(), varName) : varName,
                cfgReader_ ? NameFormat::valueName(cfgReader_->format(), type) : type,
                argListValue);
    }

    auto& operator<<(const std::string& info)
    {
        argList_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        argList_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        argList_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator <<(std::function<void(const TArgList&)> validationFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(std::make_unique<Validator<TArgList>>(*argList_, argListValue_, std::move(validationFunc)));
        return *this;
    }

    auto& operator()(TArgList defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        argList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator TArgList()
    {
        if (cfgReader_)
            cfgReader_->setArgList(std::move(argList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ArgList<TArgList>> argList_;
    TArgList defaultValue_;
    ConfigReaderPtr cfgReader_;
    TArgList& argListValue_;
};

}