#pragma once
#include <memory>

namespace cmdlime::detail{
class IParam;
class IFlag;
class IArg;
class IArgList;

template<typename TConfig>
class ConfigAccess{
public:
    ConfigAccess(TConfig& config)
        : config_(config)
    {}

    void addParam(std::unique_ptr<IParam> param)
    {
        config_.addParam(std::move(param));
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

