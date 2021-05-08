#pragma once
#include <string>

namespace cmdlime{

class Name{
public:
    Name(const std::string& name = {})
        : value_(name)
    {}

    const std::string& value() const
    {
        return value_;
    }

private:
    std::string value_;
};

class ShortName{
public:
    ShortName(const std::string& name = {})
        : value_(name)
    {}

    const std::string& value() const
    {
        return value_;
    }

private:
    std::string value_;
};

}
