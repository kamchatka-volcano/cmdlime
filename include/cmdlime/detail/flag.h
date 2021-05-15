#pragma once
#include "iflag.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include "customnames.h"
#include <memory>
#include <functional>

namespace cmdlime::detail{

class Flag : public IFlag, public ConfigVar{
public:
    Flag(const std::string& name,
         const std::string& shortName,
         std::function<bool&()> flagGetter)
        : ConfigVar(name, shortName, {})
        , flagGetter_(flagGetter)
    {
    }

private:
    ConfigVar& info() override
    {
        return *this;
    }

    const ConfigVar& info() const override
    {
        return *this;
    }

    void set() override
    {
        flagGetter_() = true;
    }

private:    
    std::function<bool&()> flagGetter_;
};

template <typename TConfig>
class FlagCreator{
    using NameProvider = typename Format<TConfig::format>::nameProvider;

public:
    FlagCreator(TConfig& cfg,
                const std::string& varName,
                std::function<bool&()> flagGetter)
        : flag_(std::make_unique<Flag>(NameProvider::name(varName),
                                       NameProvider::shortName(varName),
                                       flagGetter))
        , cfg_(cfg)
    {}

    FlagCreator& operator<<(const std::string& info)
    {
        flag_->addDescription(info);
        return *this;
    }

    FlagCreator& operator<<(const Name& customName)
    {
        flag_->resetName(customName.value());
        return *this;
    }

    FlagCreator& operator<<(const ShortName& customName)
    {
        static_assert(Format<TConfig::format>::shortNamesEnabled, "Current command line format doesn't support short names");
        flag_->resetShortName(customName.value());
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


