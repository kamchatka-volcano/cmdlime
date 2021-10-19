#pragma once
#include "optioninfo.h"

namespace cmdlime::detail{

class IFlag{
public:    
    virtual ~IFlag() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual void set() = 0;
    virtual bool isSet() const = 0;
    virtual bool isExitFlag() const = 0;
};

}
