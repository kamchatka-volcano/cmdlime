#pragma once
#include <string>
#include "configvar.h"

namespace cmdlime::detail{

class IParam{
public:    
    virtual ~IParam() = default;
    virtual ConfigVar& info() = 0;
    virtual const ConfigVar& info() const = 0;
    virtual void read(const std::string& data)  = 0;
    virtual bool hasValue() const = 0;
    virtual bool isOptional() const = 0;
    virtual std::string defaultValue() const = 0;
};

}
