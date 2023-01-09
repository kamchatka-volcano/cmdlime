#ifndef CMDLIME_IOPTION_H
#define CMDLIME_IOPTION_H

#include "external/sfun/interface.h"
#include <string>

namespace cmdlime::detail {
class OptionInfo;

enum class OptionType {
    Arg,
    ArgList,
    Command,
    Subcommand,
    Flag,
    ExitFlag,
    Param,
    ParamList
};

class IOption : private sfun::Interface<IOption> {
public:
    virtual OptionInfo& info() = 0;
    virtual const OptionInfo& info() const = 0;
    virtual OptionType type() const = 0;
};
} //namespace cmdlime::detail

#endif //CMDLIME_IOPTION_H