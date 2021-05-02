#pragma once
#include "configvar.h"

namespace cmdlime::detail{

class IFlag{
public:    
    virtual ~IFlag() = default;
    virtual ConfigVar& info() = 0;
    virtual const ConfigVar& info() const = 0;
    virtual void set() = 0;
};

}
