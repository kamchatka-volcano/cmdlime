#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "format.h"
#include "errors.h"
#include "gsl/pointers"
#include <vector>
#include <deque>
#include <unordered_set>
#include <algorithm>

namespace cmdlime::detail{
using namespace gsl;

template <FormatType formatType>
class Parser{
    using OutputFormatter = typename Format<formatType>::outputFormatter;

public:
    Parser(const std::vector<not_null<IParam*>>& params,
           const std::vector<not_null<IParamList*>>& paramLists,
           const std::vector<not_null<IFlag*>>& flags,
           const std::vector<not_null<IArg*>>& args,
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
    enum class FindMode{
        Name,
        ShortName,
        All
    };
    IParam* findParam(const std::string& name, FindMode mode = FindMode::All)
    {
        auto paramIt = std::find_if(params_.begin(), params_.end(),
            [&](auto param){
                switch(mode){
                case FindMode::Name:
                    return param->info().name() == name;
                case FindMode::ShortName:
                    return param->info().shortName() == name;
                case FindMode::All:
                    return param->info().name() == name || param->info().shortName() == name;
                }
            });
        if (paramIt == params_.end())
            return nullptr;
        return *paramIt;
    }

    IParamList* findParamList(const std::string& name, FindMode mode = FindMode::All)
    {
        auto paramListIt = std::find_if(paramLists_.begin(), paramLists_.end(),
            [&](auto paramList){
                switch(mode){
                case FindMode::Name:
                    return paramList->info().name() == name;
                case FindMode::ShortName:
                    return paramList->info().shortName() == name;
                case FindMode::All:
                    return paramList->info().name() == name || paramList->info().shortName() == name;
                }
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
            if (!param->read(value))
                throw ParsingError{"Couldn't set parameter '" + OutputFormatter::paramPrefix() + param->info().name() + "' value from '" + value + "'"};
            return;
        }
        auto paramList = findParamList(name);
        if (paramList){
            if (!paramList->read(value))
                throw ParsingError{"Couldn't set parameter '" + OutputFormatter::paramPrefix() + paramList->info().name() + "' value from '" + value + "'"};;
            return;
        }
        throw ParsingError{"Encountered unknown parameter '" + OutputFormatter::paramPrefix() + name + "'"};
    }

    IFlag* findFlag(const std::string& name, FindMode mode = FindMode::All)
    {
        auto flagIt = std::find_if(flags_.begin(), flags_.end(),
            [&](auto flag){
            switch(mode){
                case FindMode::Name:
                    return flag->info().name() == name;
                case FindMode::ShortName:
                    return flag->info().shortName() == name;
                case FindMode::All:
                    return flag->info().name() == name || flag->info().shortName() == name;
                }
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
                throw ParsingError{"Arg '" + arg->info().name() + "' value can't be empty"};
            argsToRead_.pop_front();
            if (!arg->read(value))
                throw ParsingError{"Couldn't set argument '" + arg->info().name() + "' value from '" + value + "'"};
        }
        else if (argList_){
            if (value.empty())
                throw ParsingError{"Arg list '" + argList_->info().name() + "' element value can't be empty"};
            if (!argList_->read(value))
                throw ParsingError{"Couldn't set argument list '" + argList_->info().name() + "' element's value from '" + value + "'"};
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
            throw ParsingError{"Positional argument '" + argsToRead_.front()->info().name() + "' is missing."};
    }

    void checkUnreadArgList()
    {
        if (argList_ && !argList_->hasValue())
            throw ParsingError{"Arguments list '" + argList_->info().name() + "' is missing."};
    }

    void checkNames()
    {
        auto encounteredNames = std::unordered_set<std::string>{};

        auto processName = [&encounteredNames](const std::string& varType, const ConfigVar& var){
            if (encounteredNames.count(var.name()))
                throw ConfigError{varType + " name '" + var.name() + "' is already used."};
            encounteredNames.insert(var.name());
            if (var.shortName().empty())
                return;
            if (encounteredNames.count(var.shortName()))
                throw ConfigError{varType + " short name '" + var.shortName() + "' is already used."};
            encounteredNames.insert(var.shortName());
        };
        for (auto param : params_)
            processName("Parameter's", param->info());
        for (auto paramList : paramLists_)
            processName("Parameter's", paramList->info());
        for (auto flag : flags_)
            processName("Flag's", flag->info());
    }

protected:
    std::vector<not_null<IParam*>> params_;
    std::vector<not_null<IParamList*>> paramLists_;
    std::vector<not_null<IFlag*>> flags_;
    std::vector<not_null<IArg*>> args_;
    std::deque<not_null<IArg*>> argsToRead_;
    IArgList* argList_;
};


}
