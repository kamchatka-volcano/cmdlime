#ifndef CMDLIME_IARGLIST_H
#define CMDLIME_IARGLIST_H

#include "ioption.h"
#include <string>

namespace cmdlime::detail{
class OptionInfo;

class IArgList : public IOption{
public:
    virtual bool read(const std::string& data)  = 0;
    virtual bool hasValue() const = 0;
    virtual bool isOptional() const = 0;
    virtual std::string defaultValue() const = 0;
};

}

#endif //CMDLIME_IARGLIST_H
