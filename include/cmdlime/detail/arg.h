#pragma once
#include "iarg.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include "streamreader.h"
#include "gsl/assert"
#include <cmdlime/errors.h>
#include <cmdlime/customnames.h>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class Arg : public IArg, public ConfigVar{
public:
    Arg(std::string name,
        std::string type,
        std::function<T&()> argGetter)
        : ConfigVar(std::move(name), {}, std::move(type))
        , argGetter_(std::move(argGetter))
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

    bool read(const std::string& data) override
    {
        auto stream = std::stringstream{data};
        return readFromStream(stream, argGetter_());
    }

private:    
    std::function<T&()> argGetter_;
};

template <>
inline bool Arg<std::string>::read(const std::string& data)
{
    argGetter_() = data;
    return true;
}

template<typename T, typename TConfig>
class ArgCreator{
    using NameProvider = typename Format<ConfigAccess<TConfig>::format()>::nameProvider;
public:
    ArgCreator(TConfig& cfg,
               const std::string& varName,
               const std::string& type,
               std::function<T&()> argGetter)        
        : cfg_(cfg)
    {
        Expects(!varName.empty());
        Expects(!type.empty());
        arg_ = std::make_unique<Arg<T>>(NameProvider::fullName(varName),
                                        NameProvider::valueName(type),
                                        std::move(argGetter));
    }

    ArgCreator<T, TConfig>& operator<<(const std::string& info)
    {
        arg_->addDescription(info);
        return *this;
    }

    ArgCreator<T, TConfig>& operator<<(const Name& customName)
    {
        arg_->resetName(customName.value());
        return *this;
    }

    ArgCreator<T, TConfig>& operator<<(const ValueName& valueName)
    {
        arg_->resetValueName(valueName.value());
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

template <typename T, typename TConfig>
ArgCreator<T, TConfig> makeArgCreator(TConfig& cfg,
                                       const std::string& varName,
                                       const std::string& type,
                                       std::function<T&()> argGetter)
{
    return ArgCreator<T, TConfig>{cfg, varName, type, std::move(argGetter)};
}


}
