#ifndef CMDLIME_ARGLISTCREATOR_H
#define CMDLIME_ARGLISTCREATOR_H

#include "arglist.h"
#include "icommandlinereader.h"
#include "nameformat.h"
#include "validator.h"
#include "external/sfun/contract.h"
#include "external/sfun/type_traits.h"

namespace cmdlime::detail {

template<typename TArgList>
class ArgListCreator {
    static_assert(sfun::is_dynamic_sequence_container_v<TArgList>, "Argument list field must be a sequence container");

public:
    ArgListCreator(
            CommandLineReaderPtr reader,
            const std::string& varName,
            const std::string& type,
            TArgList& argListValue)
        : reader_(reader)
        , argListValue_(argListValue)
    {
        sfunPrecondition(!varName.empty());
        sfunPrecondition(!type.empty());
        argList_ = std::make_unique<ArgList<TArgList>>(
                reader_ ? NameFormat::fullName(reader_->format(), varName) : varName,
                reader_ ? NameFormat::valueName(reader_->format(), type) : type,
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

    auto& operator<<(std::function<void(const TArgList&)> validationFunc)
    {
        if (reader_)
            reader_->addValidator(
                    std::make_unique<Validator<TArgList>>(*argList_, argListValue_, std::move(validationFunc)));
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
        if (reader_)
            reader_->setArgList(std::move(argList_));
        return defaultValue_;
    }

private:
    std::unique_ptr<ArgList<TArgList>> argList_;
    TArgList defaultValue_;
    CommandLineReaderPtr reader_;
    TArgList& argListValue_;
};

} //namespace cmdlime::detail
#endif //CMDLIME_ARGLISTCREATOR_H