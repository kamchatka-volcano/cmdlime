#pragma once
#include <string>

namespace cmdlime::detail{
class OptionInfo;

class IArg{
public:    
    virtual ~IArg() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual bool read(const std::string& data)  = 0;
};

}
