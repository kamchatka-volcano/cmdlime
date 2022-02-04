#pragma once
#include <string>

namespace cmdlime::detail{
class OptionInfo;

class IArgList{
public:    
    virtual ~IArgList() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual bool read(const std::string& data)  = 0;
    virtual bool hasValue() const = 0;
    virtual bool isOptional() const = 0;
    virtual std::string defaultValue() const = 0;
};

}
