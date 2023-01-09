#ifndef CMDLIME_PARAMCREATOR_H
#define CMDLIME_PARAMCREATOR_H

#include "icommandlinereader.h"
#include "nameformat.h"
#include "param.h"
#include "validator.h"
#include "external/sfun/contract.h"

namespace cmdlime::detail {

template<typename T>
class ParamCreator {
public:
    ParamCreator(CommandLineReaderPtr reader, const std::string& varName, const std::string& type, T& paramValue)
        : reader_(reader)
        , paramValue_(paramValue)
    {
        sfunPrecondition(!varName.empty());
        sfunPrecondition(!type.empty());
        param_ = std::make_unique<Param<T>>(
                reader_ ? NameFormat::name(reader->format(), varName) : varName,
                reader_ ? NameFormat::shortName(reader->format(), varName) : varName,
                reader_ ? NameFormat::valueName(reader->format(), type) : varName,
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
        if (reader_ && reader_->shortNamesEnabled())
            param_->info().resetShortName(customName.value());
        return *this;
    }

    auto& operator<<(const WithoutShortName&)
    {
        if (reader_ && reader_->shortNamesEnabled())
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
        if (reader_)
            reader_->addValidator(std::make_unique<Validator<T>>(*param_, paramValue_, std::move(validationFunc)));
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
        if (reader_)
            reader_->addParam(std::move(param_));
        return defaultValue_;
    }

private:
    std::unique_ptr<Param<T>> param_;
    T defaultValue_;
    CommandLineReaderPtr reader_;
    T& paramValue_;
};

} //namespace cmdlime::detail

#endif //CMDLIME_PARAMCREATOR_H