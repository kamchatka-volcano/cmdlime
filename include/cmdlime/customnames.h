#pragma once
#include "errors.h"
#include <string>
#include <utility>

namespace cmdlime{

namespace detail {
enum class CustomNameType {
    Name,
    ShortName,
    ValueName
};
}

template <detail::CustomNameType>
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

using Name = CustomName<detail::CustomNameType::Name>;
using ShortName = CustomName<detail::CustomNameType::ShortName>;
using ValueName = CustomName<detail::CustomNameType::ValueName>;
class WithoutShortName{};

}
