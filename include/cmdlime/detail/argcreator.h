#ifndef CMDLIME_ARGCREATOR_H
#define CMDLIME_ARGCREATOR_H

#include "arg.h"
#include "icommandlinereader.h"
#include "nameformat.h"
#include "validator.h"
#include "external/sfun/precondition.h"

namespace cmdlime::detail {

template<typename T>
class ArgCreator {
public:
    ArgCreator(
            CommandLineReaderPtr reader,
            sfun::not_empty<const std::string&> varName,
            sfun::not_empty<const std::string&> type,
            T& argValue)
        : reader_(reader)
        , argValue_(argValue)
    {
        arg_ = std::make_unique<Arg<T>>(
                reader_ ? NameFormat::fullName(reader_->format(), varName.get()) : varName.get(),
                reader_ ? NameFormat::valueName(reader_->format(), type.get()) : type.get(),
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
        if (reader_)
            reader_->addValidator(std::make_unique<Validator<T>>(*arg_, argValue_, std::move(validationFunc)));
        return *this;
    }

    operator T()
    {
        if (reader_)
            reader_->addArg(std::move(arg_));
        return T{};
    }

private:
    std::unique_ptr<Arg<T>> arg_;
    CommandLineReaderPtr reader_;
    T& argValue_;
};

} //namespace cmdlime::detail

#endif //CMDLIME_ARGCREATOR_H