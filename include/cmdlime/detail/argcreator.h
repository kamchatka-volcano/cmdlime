#pragma once
#include "arg.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include <gsl/assert>

namespace cmdlime::detail{

template<typename T>
class ArgCreator{
public:
    ArgCreator(ConfigReaderPtr cfgReader,
               const std::string& varName,
               const std::string& type,
               T& argValue)
            : cfgReader_(cfgReader)
            , argValue_(argValue)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        arg_ = std::make_unique<Arg<T>>(
                cfgReader_ ? NameFormat::fullName(cfgReader_->format(), varName) : varName,
                cfgReader_ ? NameFormat::valueName(cfgReader_->format(), type) : type,
                argValue);
    }

    auto& operator<<(const std::string& info)
    {
        arg_->info().addDescription(info);
        return *this;
    }

    auto& operator<<(const Name& customName)
    {
        arg_->info().resetName(customName.value());
        return *this;
    }

    auto& operator<<(const ValueName& valueName)
    {
        arg_->info().resetValueName(valueName.value());
        return *this;
    }

    auto& operator<<(std::function<void(const T&)> validationFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(std::make_unique<Validator<T>>(*arg_, argValue_, std::move(validationFunc)));
        return *this;
    }

    operator T()
    {
        if (cfgReader_)
            cfgReader_->addArg(std::move(arg_));
        return T{};
    }

private:
    std::unique_ptr<Arg<T>> arg_;
    ConfigReaderPtr cfgReader_;
    T& argValue_;
};

}