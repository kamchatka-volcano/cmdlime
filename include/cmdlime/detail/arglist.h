#pragma once
#include "iarglist.h"
#include "configvar.h"
#include "configaccess.h"
#include "format.h"
#include <vector>
#include <sstream>
#include <functional>
#include <memory>

namespace cmdlime::detail{

template <typename T>
class ArgList : public IArgList, public ConfigVar{
public:
    ArgList(const std::string& name, const std::string& shortName, const std::string& type, std::function<std::vector<T>&()> argListGetter)
        : ConfigVar(name, shortName, type)
        , argListGetter_(argListGetter)
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
        argListGetter_().emplace_back();
        stream >> argListGetter_().back();
    }

private:        
    std::function<std::vector<T>&()> argListGetter_;
};

template<typename T, typename TConfig>
class ArgListCreator{
    using NameProvider = typename Format<TConfig::format>::nameProvider;

public:
    ArgListCreator(TConfig& cfg,
                   const std::string& varName,
                   const std::string& type,
                   std::function<std::vector<T>&()> argListGetter)
        : argList_(std::make_unique<ArgList<T>>(NameProvider::name(varName),
                                                NameProvider::shortName(varName),
                                                type, argListGetter))
        , cfg_(cfg)
    {}

    ArgListCreator<T, TConfig>& operator<<(const std::string& info)
    {
        argList_->addDescription(info);
        return *this;
    }

    operator std::vector<T>()
    {
        ConfigAccess<TConfig>{cfg_}.setArgList(std::move(argList_));
        return std::vector<T>{};
    }

private:
    std::unique_ptr<ArgList<T>> argList_;
    TConfig& cfg_;
};


}
