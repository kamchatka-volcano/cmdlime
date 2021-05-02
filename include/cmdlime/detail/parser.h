#pragma once
#include "iparam.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "format.h"
#include <vector>
#include <deque>
#include <algorithm>

namespace cmdlime::detail{

template <FormatType formatType>
class Parser{
    using OutputFormatter = typename Format<formatType>::outputFormatter;

public:
    Parser(const std::vector<IParam*>& params,
           const std::vector<IFlag*>& flags,
           const std::vector<IArg*>& args,
           IArgList* argList)
        : params_(params)
        , flags_(flags)
        , args_(args)
        , argList_(argList)
    {}
    virtual ~Parser() = default;

    void parse(const std::vector<std::string>& cmdLine)
    {
        argsToRead_.clear();
        std::copy(args_.begin(), args_.end(), std::back_inserter(argsToRead_));
        process(cmdLine);
        checkUnreadParams();
        if(!argsToRead_.empty())
            throw std::runtime_error{"Positional argument " + argsToRead_.front()->info().name() + " is missing."};
    }

protected:
    void readParam(const std::string& name, const std::string& value)
    {
        auto paramIt = std::find_if(params_.begin(), params_.end(), [name](auto param){return param->info().name() == name;});
        if (paramIt == params_.end())
            throw std::runtime_error("Encountered unrecognized param " + OutputFormatter::paramPrefix() + name);
        (*paramIt)->read(value);
    }

    void readFlag(const std::string& name)
    {
        auto flagIt = std::find_if(flags_.begin(), flags_.end(), [name](auto flag){return flag->info().name() == name;});
        if (flagIt == flags_.end())
            throw std::runtime_error{"Encountered unrecognized flag " + OutputFormatter::flagPrefix() + name};
        (*flagIt)->set();
    }

    void readArg(const std::string& value)
    {
        if (!argsToRead_.empty()){
            auto arg = argsToRead_.front();
            argsToRead_.pop_front();
            arg->read(value);
        }
        else if (argList_)
            argList_->read(value);
        else
            throw std::runtime_error("Encountered unrecognized positional argument " + value);
    }

private:
    virtual void process(const std::vector<std::string>& cmdLine) = 0;

    void checkUnreadParams()
    {
        for (const auto& param : params_)
            if (!param->hasValue())
                throw std::runtime_error{"Param " + OutputFormatter::paramPrefix() + param->info().name() + " is missing."};
    }


protected:
    std::vector<IParam*> params_;
    std::vector<IFlag*> flags_;
    std::vector<IArg*> args_;
    std::deque<IArg*> argsToRead_;
    IArgList* argList_;
};


}
