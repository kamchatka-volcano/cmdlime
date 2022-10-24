#ifndef CMDLIME_IARG_H
#define CMDLIME_IARG_H

#include "ioption.h"
#include <string>

namespace cmdlime::detail{

class IArg : public IOption{
public:
    virtual bool read(const std::string& data)  = 0;
};

}

#endif //CMDLIME_IARG_H