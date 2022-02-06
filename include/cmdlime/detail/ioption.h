#pragma once

namespace cmdlime::detail{
class OptionInfo;

class IOption{
public:
    virtual ~IOption() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
};
}