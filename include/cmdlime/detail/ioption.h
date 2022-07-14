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
    IOption() = default;
    virtual ~IOption() = default;
    IOption(const OptionInfo& info) = delete;
    IOption& operator=(const OptionInfo& info) = delete;
    IOption(OptionInfo&& info) = delete;
    IOption& operator=(OptionInfo&& info) = delete;

    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual OptionType type() const = 0;
};
}