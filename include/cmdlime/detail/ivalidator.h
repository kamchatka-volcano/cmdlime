#ifndef CMDLIME_IVALIDATOR_H
#define CMDLIME_IVALIDATOR_H

#include "ioption.h"
#include "external/eel/interface.h"

namespace cmdlime::detail {

class IValidator : private eel::interface<IValidator> {
public:
    virtual void validate(const std::string& commandName) const = 0;
    virtual OptionType optionType() const = 0;
};

} //namespace cmdlime::detail

#endif //CMDLIME_IVALIDATOR_H