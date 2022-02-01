#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "icommand.h"
#include <vector>
#include <memory>

namespace cmdlime::detail{

class Options {
public:
    const std::vector<std::unique_ptr<IParam>>& params() const
    {
        return params_;
    }

    const std::vector<std::unique_ptr<IParamList>>& paramLists() const
    {
        return paramLists_;
    }

    const std::vector<std::unique_ptr<IFlag>>& flags() const
    {
        return flags_;
    }

    const std::vector<std::unique_ptr<IArg>>& args() const
    {
        return args_;
    }

    IArgList* argList() const
    {
        return argList_.get();
    }

    const std::vector<std::unique_ptr<ICommand>>& commands() const
    {
        return commands_;
    }

    void addParam(std::unique_ptr<IParam> param)
    {
        params_.emplace_back(std::move(param));
    }

    void addParamList(std::unique_ptr<IParamList> paramList)
    {
        paramLists_.emplace_back(std::move(paramList));
    }

    void addFlag(std::unique_ptr<IFlag> flag)
    {
        flags_.emplace_back(std::move(flag));
    }

    void addArg(std::unique_ptr<IArg> arg)
    {
        args_.emplace_back(std::move(arg));
    }

    void setArgList(std::unique_ptr<IArgList> argList)
    {
        argList_ = std::move(argList);
    }

    void addCommand(std::unique_ptr<ICommand> command)
    {
        commands_.emplace_back(std::move(command));
    }

private:
    std::vector<std::unique_ptr<IParam>> params_;
    std::vector<std::unique_ptr<IParamList>> paramLists_;
    std::vector<std::unique_ptr<IFlag>> flags_;
    std::vector<std::unique_ptr<IArg>> args_;
    std::unique_ptr<IArgList> argList_;
    std::vector<std::unique_ptr<ICommand>> commands_;
};

}
