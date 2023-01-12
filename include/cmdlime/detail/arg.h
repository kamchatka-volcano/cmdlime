#ifndef CMDLIME_ARG_H
#define CMDLIME_ARG_H

#include "iarg.h"
#include "optioninfo.h"
#include <cmdlime/customnames.h>
#include <cmdlime/errors.h>
#include <cmdlime/stringconverter.h>
#include <functional>
#include <memory>
#include <sstream>

namespace cmdlime::detail {

template<typename T>
class Arg : public IArg {
public:
    Arg(std::string name, std::string type, T& argValue)
        : info_(std::move(name), {}, std::move(type))
        , argValue_(argValue)
    {
    }

    OptionInfo& info() override
    {
        return info_;
    }

    const OptionInfo& info() const override
    {
        return info_;
    }

    OptionType type() const override
    {
        return OptionType::Arg;
    }

private:
    void read(const std::string& data) override
    {
        argValue_ = convertFromString<T>(data);
    }

private:
    OptionInfo info_;
    T& argValue_;
};

} //namespace cmdlime::detail

#endif //CMDLIME_ARG_H
