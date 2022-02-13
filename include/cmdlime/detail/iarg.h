#pragma once
#include "ioption.h"
#include <string>

namespace cmdlime::detail{

class IArg : public IOption{
public:
    virtual bool read(const std::string& data)  = 0;
};

}
