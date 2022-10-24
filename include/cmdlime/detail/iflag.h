#ifndef CMDLIME_IFLAG_H
#define CMDLIME_IFLAG_H

#include "ioption.h"

namespace cmdlime::detail{
class OptionInfo;

class IFlag : public IOption{
public:
    virtual void set() = 0;
    virtual bool isSet() const = 0;
    virtual bool isExitFlag() const = 0;
};

}

#endif //CMDLIME_IFLAG_H