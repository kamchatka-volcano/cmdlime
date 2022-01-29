#pragma once
#include "flag.h"
#include "configaccess.h"
#include "format.h"

namespace cmdlime::detail{

template <typename TConfig>
class FlagCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;

public:
    FlagCreator(TConfig& cfg,
                const std::string& varName,
                bool& flagValue,
                Flag::Type flagType = Flag::Type::Normal)
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        flag_ = std::make_unique<Flag>(NameProvider::name(varName),
                                       NameProvider::shortName(varName),
                                       flagValue,
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

template <typename TConfig>
FlagCreator<TConfig> makeFlagCreator(TConfig& cfg,
                                     const std::string& varName,
                                     const std::function<bool&()>& flagGetter,
                                     Flag::Type flagType = Flag::Type::Normal)
{
    return FlagCreator<TConfig>{cfg, varName, flagGetter(), flagType};
}

}