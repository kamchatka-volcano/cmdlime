#pragma once
#include "format.h"
#include <memory>

namespace cmdlime::detail{
class IParam;
class IParamList;
class IFlag;
class IArg;
class IArgList;

template<typename TConfig>
class ConfigAccess{
public:
    ConfigAccess(TConfig& config)
        : config_(config)
    {}

    static constexpr FormatType format()
    {
        return TConfig::format;
    }

    void addParam(std::unique_ptr<IParam> param)
    {
        config_.addParam(std::move(param));
    }

    void addParamList(std::unique_ptr<IParamList> param)
    {
        config_.addParamList(std::move(param));
    }

    void addFlag(std::unique_ptr<IFlag> flag)
    {
        config_.addFlag(std::move(flag));
    }

    void addArg(std::unique_ptr<IArg> arg)
    {
        config_.addArg(std::move(arg));
    }

    void setArgList(std::unique_ptr<IArgList> argList)
    {
        config_.setArgList(std::move(argList));
    }

private:
    TConfig& config_;
};

}

