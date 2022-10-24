#ifndef CMDLIME_IPARAMLIST_H
#define CMDLIME_IPARAMLIST_H

#include "ioption.h"
#include <string>

namespace cmdlime::detail{
class OptionInfo;

class IParamList : public IOption{
public:
    virtual bool read(const std::string& data)  = 0;
    virtual bool hasValue() const = 0;
    virtual bool isOptional() const = 0;
    virtual std::string defaultValue() const = 0;
};

}

#endif //CMDLIME_IPARAMLIST_H
