#ifndef CMDLIME_IVALIDATOR_H
#define CMDLIME_IVALIDATOR_H

#include "ioption.h"
#include "external/sfun/interface.h"

namespace cmdlime::detail {

class IValidator : private sfun::Interface<IValidator>{
public:
    virtual void validate(const std::string& commandName) const = 0;
    virtual OptionType optionType() const = 0;
};

}

#endif //CMDLIME_IVALIDATOR_H