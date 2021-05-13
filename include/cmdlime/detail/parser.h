#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "format.h"
#include "errors.h"
#include <vector>
#include <deque>
#include <unordered_set>
#include <algorithm>

namespace cmdlime::detail{

template <FormatType formatType>
class Parser{
    using OutputFormatter = typename Format<formatType>::outputFormatter;

public:
    Parser(const std::vector<IParam*>& params,
           const std::vector<IParamList*>& paramLists,
           const std::vector<IFlag*>& flags,
           const std::vector<IArg*>& args,
           IArgList* argList)
        : params_(params)
        , paramLists_(paramLists)
        , flags_(flags)
        , args_(args)
        , argList_(argList)
    {}
    virtual ~Parser() = default;

    void parse(const std::vector<std::string>& cmdLine)
    {
        checkNames();
        argsToRead_.clear();
        std::copy(args_.begin(), args_.end(), std::back_inserter(argsToRead_));

        auto argsDelimiterIt = std::find(cmdLine.begin(), cmdLine.end(), "--");
        auto cmdLineBeforeDelimiter = std::vector<std::string>(cmdLine.begin(), argsDelimiterIt);
        process(cmdLineBeforeDelimiter);

        if (argsDelimiterIt != cmdLine.end()){
            auto argsAfterDelimiter = std::vector<std::string>(argsDelimiterIt + 1, cmdLine.end());
            for (const auto& arg : argsAfterDelimiter)
                readArg(arg);
        }

        checkUnreadParams();
        checkUnreadArgs();
        checkUnreadArgList();
    }

protected:
    IParam* findParam(const std::string& name)
    {
        auto paramIt = std::find_if(params_.begin(), params_.end(),
            [&name](auto param){
                return param->info().name() == name || param->info().shortName() == name;
            });
        if (paramIt == params_.end())
            return nullptr;
        return *paramIt;
    }

    IParamList* findParamList(const std::string& name)
    {
        auto paramListIt = std::find_if(paramLists_.begin(), paramLists_.end(),
            [&name](auto paramList){
                return paramList->info().name() == name || paramList->info().shortName() == name;
            });
        if (paramListIt == paramLists_.end())
            return nullptr;
        return *paramListIt;
    }

    void readParam(const std::string& name, std::string value)
    {
        if (value.empty())
            throw ParsingError{"Parameter '" + OutputFormatter::paramPrefix() + name + "' value can't be empty"};
        auto param = findParam(name);
        if (param){
            param->read(value);
            return;
        }
        auto paramList = findParamList(name);
        if (paramList){
            paramList->read(value);
            return;
        }
        throw ParsingError{"Encountered unknown parameter '" + OutputFormatter::paramPrefix() + name + "'"};
    }

    IFlag* findFlag(const std::string& name)
    {
        auto flagIt = std::find_if(flags_.begin(), flags_.end(),
            [&name](auto flag){
                return flag->info().name() == name || flag->info().shortName() == name;
            });
        if (flagIt == flags_.end())
            return nullptr;
        return *flagIt;
    }

    void readFlag(const std::string& name)
    {
        auto flag = findFlag(name);
        if (!flag)
            throw ParsingError{"Encountered unknown flag '" + OutputFormatter::flagPrefix() + name + "'"};
        flag->set();
    }

    void readArg(const std::string& value)
    {
        if (!argsToRead_.empty()){
            auto arg = argsToRead_.front();
            if (value.empty())
                throw ParsingError{"Arg '" + OutputFormatter::argName(*arg) + "' value can't be empty"};
            argsToRead_.pop_front();
            arg->read(value);
        }
        else if (argList_){
            if (value.empty())
                throw ParsingError{"Arg list '" + OutputFormatter::argListName(*argList_) + "' element value can't be empty"};
            argList_->read(value);
        }
        else
            throw ParsingError("Encountered unknown positional argument '" + value + "'");
    }

private:
    virtual void process(const std::vector<std::string>& cmdLine) = 0;

    void checkUnreadParams()
    {
        for (const auto param : params_)
            if (!param->hasValue())
                throw ParsingError{"Parameter '" + OutputFormatter::paramPrefix() + param->info().name() + "' is missing."};

        for (const auto paramList : paramLists_)
            if (!paramList->hasValue())
                throw ParsingError{"Parameter '" + OutputFormatter::paramPrefix() + paramList->info().name() + "' is missing."};
    }

    void checkUnreadArgs()
    {
        if(!argsToRead_.empty())
            throw ParsingError{"Positional argument '" + OutputFormatter::argName(*argsToRead_.front()) + "' is missing."};
    }

    void checkUnreadArgList()
    {
        if (argList_ && !argList_->hasValue())
            throw ParsingError{"Arguments list '" + OutputFormatter::argListName(*argList_) + "' is missing."};
    }

    void checkNames()
    {
        std::unordered_set<std::string> names;
        auto processName = [&names](const std::string& varType, const ConfigVar& var){
            if (var.name().empty())
                throw ConfigError{varType + " name '" + var.name() + "' can't be empty."};
            if (names.count(var.name()))
                throw ConfigError{varType + " name '" + var.name() + "' is already used."};
            names.insert(var.name());
            if (var.shortName().empty())
                return;
            if (names.count(var.shortName()))
                throw ConfigError{varType + " short name '" + var.shortName() + "' is already used."};
            names.insert(var.shortName());
        };
        for (auto param : params_)
            processName("Parameter's", param->info());
        for (auto paramList : paramLists_)
            processName("Parameter's", paramList->info());
        for (auto flag : flags_)
            processName("Flag's", flag->info());
    }

protected:
    std::vector<IParam*> params_;
    std::vector<IParamList*> paramLists_;
    std::vector<IFlag*> flags_;
    std::vector<IArg*> args_;
    std::deque<IArg*> argsToRead_;
    IArgList* argList_;
};


}
