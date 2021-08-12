#pragma once
#include "format.h"
#include "gsl/pointers"
#include <memory>
#include <vector>

namespace cmdlime::detail{
using namespace gsl;
class IParam;
class IParamList;
class IFlag;
class IArg;
class IArgList;
class ICommand;

template<typename TConfig>
class ConfigAccess{
public:
    explicit ConfigAccess(TConfig& config)
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

    void addCommand(std::unique_ptr<ICommand> command)
    {
        config_.addCommand(std::move(command));
    }

    std::vector<not_null<ICommand*>> commandList()
    {
        return config_.commandList();
    }

    void addHelpFlagToCommands(const std::string& commandName)
    {
        config_.addHelpFlagToCommands(commandName);
    }

private:
    TConfig& config_;
};

}

