#ifndef CMDLIME_FLAGCREATOR_H
#define CMDLIME_FLAGCREATOR_H

#include "flag.h"
#include "icommandlinereader.h"
#include "nameformat.h"
#include "validator.h"
#include "external/sfun/precondition.h"

namespace cmdlime::detail {

class FlagCreator {

public:
    FlagCreator(
            CommandLineReaderPtr reader,
            sfun::not_empty<const std::string&> varName,
            bool& flagValue,
            Flag::Type flagType = Flag::Type::Normal)
        : reader_(reader)
    {
        flag_ = std::make_unique<Flag>(
                reader_ ? NameFormat::name(reader_->format(), varName.get()) : varName.get(),
                reader_ ? NameFormat::shortName(reader_->format(), varName.get()) : varName.get(),
                flagValue,
                flagType);
    }

    FlagCreator& operator<<(const std::string& info)
    {
        flag_->info().addDescription(info);
        return *this;
    }

    FlagCreator& operator<<(const Name& customName)
    {
        flag_->info().resetName(customName.value());
        return *this;
    }

    FlagCreator& operator<<(const ShortName& customName)
    {
        if (reader_ && reader_->shortNamesEnabled())
            flag_->info().resetShortName(customName.value());
        return *this;
    }

    FlagCreator& operator<<(const WithoutShortName&)
    {
        if (reader_ && reader_->shortNamesEnabled())
            flag_->info().resetShortName({});
        return *this;
    }

    operator bool()
    {
        if (reader_)
            reader_->addFlag(std::move(flag_));
        return false;
    }

private:
    std::unique_ptr<Flag> flag_;
    CommandLineReaderPtr reader_;
};

} //namespace cmdlime::detail

#endif //CMDLIME_FLAGCREATOR_H