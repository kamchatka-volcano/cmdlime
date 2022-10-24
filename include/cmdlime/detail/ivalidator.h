#ifndef CMDLIME_IVALIDATOR_H
#define CMDLIME_IVALIDATOR_H
#include "ioption.h"

namespace cmdlime::detail {

class IValidator{
public:
    IValidator() = default;
    virtual ~IValidator() = default;
    IValidator(const IValidator&) = delete;
    IValidator& operator=(const IValidator&) = delete;
    IValidator(IValidator&&) = delete;
    IValidator& operator=(IValidator&&) = delete;

    virtual void validate(const std::string& commandName) const = 0;
    virtual OptionType optionType() const = 0;
};

}

#endif //CMDLIME_IVALIDATOR_H