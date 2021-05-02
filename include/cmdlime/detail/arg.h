#pragma once
#include "iarg.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class Arg : public IArg, public ConfigVar{
public:
    Arg(const std::string& name, const std::string& shortName, const std::string& type, std::function<T&()> argGetter)
        : ConfigVar(name, shortName, type)
        , argGetter_(argGetter)
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

    void read(const std::string& data) override
    {
        auto stream = std::stringstream{data};
        stream >> argGetter_();
    }

private:    
    std::function<T&()> argGetter_;
};

template<typename T, typename TConfig>
class ArgCreator{
    using NameProvider = typename Format<TConfig::format>::nameProvider;
public:
    ArgCreator(TConfig& cfg,
               const std::string& varName,
               const std::string& type,
               std::function<T&()> argGetter)
        : arg_(std::make_unique<Arg<T>>(NameProvider::name(varName),
                                        NameProvider::shortName(varName),
                                        type, argGetter))
        , cfg_(cfg)
    {}

    ArgCreator<T, TConfig>& operator<<(const std::string& info)
    {
        arg_->addDescription(info);
        return *this;
    }

    operator T()
    {
        ConfigAccess<TConfig>{cfg_}.addArg(std::move(arg_));
        return T{};
    }

private:
    std::unique_ptr<Arg<T>> arg_;
    TConfig& cfg_;
};

}
