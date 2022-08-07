#pragma once
#include "flag.h"
#include "iconfigreader.h"
#include "nameformat.h"
#include "validator.h"
#include "gsl_assert.h"

namespace cmdlime::detail{

class FlagCreator{

public:
    FlagCreator(ConfigReaderPtr cfgReader,
                const std::string& varName,
                bool& flagValue,
                Flag::Type flagType = Flag::Type::Normal)
        : cfgReader_(cfgReader)
    {
        Expects(!varName.empty());
        flag_ = std::make_unique<Flag>(
                cfgReader_ ? NameFormat::name(cfgReader_->format(), varName) : varName,
                cfgReader_ ? NameFormat::shortName(cfgReader_->format(), varName) : varName,
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
        if (cfgReader_ && cfgReader_->shortNamesEnabled())
            flag_->info().resetShortName(customName.value());
        return *this;
    }

    FlagCreator& operator<<(const WithoutShortName&)
    {
        if (cfgReader_ && cfgReader_->shortNamesEnabled())
            flag_->info().resetShortName({});
        return *this;
    }

    operator bool()
    {
        if (cfgReader_)
            cfgReader_->addFlag(std::move(flag_));
        return false;
    }

private:
    std::unique_ptr<Flag> flag_;
    ConfigReaderPtr cfgReader_;
};

}