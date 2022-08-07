#pragma once
#include "arglist.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail{

template<typename T>
class ArgListCreator{

public:
    ArgListCreator(ConfigReaderPtr cfgReader,
                   const std::string& varName,
                   const std::string& type,
                   std::vector<T>& argListValue)
            : cfgReader_(cfgReader)
            , argListValue_(argListValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        argList_ = std::make_unique<ArgList<T>>(
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

    auto& operator <<(std::function<void(const std::vector<T>&)> validationFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(std::make_unique<Validator<std::vector<T>>>(*argList_, argListValue_, std::move(validationFunc)));
        return *this;
    }

    auto& operator()(std::vector<T> defaultValue = {})
    {
        defaultValue_ = std::move(defaultValue);
        argList_->setDefaultValue(defaultValue_);
        return *this;
    }

    operator std::vector<T>()
    {
        if (cfgReader_)
            cfgReader_->setArgList(std::move(argList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ArgList<T>> argList_;
    std::vector<T> defaultValue_;
    ConfigReaderPtr cfgReader_;
    std::vector<T>& argListValue_;
};

}