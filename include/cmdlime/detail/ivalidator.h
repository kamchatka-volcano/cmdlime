#pragma once
#include "ioption.h"

namespace cmdlime::detail {

class IValidator{
public:
    virtual ~IValidator() = default;
    virtual void validate(const std::string& commandName) const = 0;
    virtual OptionType optionType() const = 0;
};

}