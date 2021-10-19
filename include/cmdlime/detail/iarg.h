#pragma once
#include <string>
#include "optioninfo.h"

namespace cmdlime::detail{
class IArg{
public:    
    virtual ~IArg() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual bool read(const std::string& data)  = 0;
};

}
