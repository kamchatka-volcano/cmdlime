#pragma once
#include "iflag.h"
#include "optioninfo.h"
#include "configaccess.h"
#include "format.h"
#include <gsl/gsl>
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
         std::function<bool&()> flagGetter,
         Type type)
        : info_(std::move(name), std::move(shortName), {})
        , flagGetter_(std::move(flagGetter))
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

private:
    void set() override
    {
        flagGetter_() = true;
    }

    bool isSet() const override
    {
        return flagGetter_();
    }

    bool isExitFlag() const override
    {
        return type_ == Type::Exit;
    }

private:
    OptionInfo info_;
    std::function<bool&()> flagGetter_;
    Type type_;
};

template <typename TConfig>
class FlagCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;

public:
    FlagCreator(TConfig& cfg,
                const std::string& varName,
                std::function<bool&()> flagGetter,
                Flag::Type flagType = Flag::Type::Normal)
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        flag_ = std::make_unique<Flag>(NameProvider::name(varName),
                                       NameProvider::shortName(varName),
                                       std::move(flagGetter),
                                       flagType);
    }

    FlagCreator& operator<<(const std::string& info)
    {
        flag_->info().addDescription(info);
        return *this;
    }

    FlagCreator& operator<<(const Name& customName)
    {
        flag_->info().resetName(customName.value());
        return *this;
    }

    FlagCreator& operator<<(const ShortName& customName)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        flag_->info().resetShortName(customName.value());
        return *this;
    }

    FlagCreator& operator<<(const WithoutShortName&)
    {
        static_assert(Format<ConfigAccess<TConfig>::format()>::shortNamesEnabled,
                      "Current command line format doesn't support short names");
        flag_->info().resetShortName({});
        return *this;
    }

    operator bool()
    {
        ConfigAccess<TConfig>{cfg_}.addFlag(std::move(flag_));
        return false;
    }

private:
    std::unique_ptr<Flag> flag_;
    TConfig& cfg_;
};
}


