#pragma once
#include <string>

namespace cmdlime::detail{
class OptionInfo;

enum class OptionType{
    Arg,
    ArgList,
    Command,
    Subcommand,
    Flag,
    ExitFlag,
    Param,
    ParamList
};

class IOption{
public:
    virtual ~IOption() = default;
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual OptionType type() const = 0;
};
}