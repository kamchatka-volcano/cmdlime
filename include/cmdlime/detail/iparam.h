#ifndef CMDLIME_IPARAM_H
#define CMDLIME_IPARAM_H

#include "ioption.h"
#include <string>

namespace cmdlime::detail {
class OptionInfo;

class IParam : public IOption {
public:
    virtual bool read(const std::string& data) = 0;
    virtual bool hasValue() const = 0;
    virtual bool isOptional() const = 0;
    virtual std::string defaultValue() const = 0;
};

} //namespace cmdlime::detail

#endif //CMDLIME_IPARAM_H