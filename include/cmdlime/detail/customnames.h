#pragma once
#include <string>

namespace cmdlime{

class CustomName{
public:
    CustomName(const std::string& name = {})
        : value_(name)
    {}

    const std::string& value() const
    {
        return value_;
    }

private:
    std::string value_;
};

class Name : public CustomName{
    using CustomName::CustomName;
};

class ShortName : public CustomName{
    using CustomName::CustomName;
};

class ValueName : public CustomName{
    using CustomName::CustomName;
};

}
