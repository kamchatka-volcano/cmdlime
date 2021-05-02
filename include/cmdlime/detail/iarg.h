#pragma once
#include <string>
#include "configvar.h"

namespace cmdlime::detail{
class IArg{
public:    
    virtual ~IArg() = default;
    virtual ConfigVar& info() = 0;
    virtual const ConfigVar& info() const = 0;
    virtual void read(const std::string& data)  = 0;
};

}
