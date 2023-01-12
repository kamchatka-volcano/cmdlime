#ifndef CMDLIME_IARG_H
#define CMDLIME_IARG_H

#include "ioption.h"
#include <string>

namespace cmdlime::detail {

class IArg : public IOption {
public:
    virtual void read(const std::string& data) = 0;
};

} //namespace cmdlime::detail

#endif //CMDLIME_IARG_H