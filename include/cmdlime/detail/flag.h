#pragma once
#include "iflag.h"
#include "optioninfo.h"
#include <cmdlime/customnames.h>
#include <memory>
#include <functional>
#include <utility>

namespace cmdlime::detail{

class Flag : public IFlag{
public:
    enum class Type{
        Normal,
        Exit
    };

    Flag(std::string name,
         std::string shortName,
         bool& flagValue,
         Type type)
        : info_(std::move(name), std::move(shortName), {})
        , flagValue_(flagValue)
        , type_(type)
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
        return isExitFlag() ? OptionType::ExitFlag : OptionType::Flag;
    }

private:
    void set() override
    {
        flagValue_ = true;
    }

    bool isSet() const override
    {
        return flagValue_;
    }

    bool isExitFlag() const override
    {
        return type_ == Type::Exit;
    }

private:
    OptionInfo info_;
    bool& flagValue_;
    Type type_;
};

}


