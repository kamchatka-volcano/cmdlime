#pragma once
#include "iarg.h"
#include "optioninfo.h"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <cmdlime/stringconverter.h>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class Arg : public IArg{
public:
    Arg(std::string name,
        std::string type,
        T& argValue)
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
    bool read(const std::string& data) override
    {
        auto argVal = convertFromString<T>(data);
        if (!argVal)
            return false;
        argValue_ = *argVal;
        return true;
    }

private:
    OptionInfo info_;
    T& argValue_;
};

}
