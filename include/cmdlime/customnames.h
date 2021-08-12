#pragma once
#include "errors.h"
#include <string>
#include <utility>

namespace cmdlime{

class CustomName{
public:
    CustomName(std::string name)
        : value_(std::move(name))
    {
        if(value_.empty())
            throw ConfigError{"Custom name can't be empty."};
    }

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

class WithoutShortName{};

}
