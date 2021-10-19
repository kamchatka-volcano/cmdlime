#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "icommand.h"
#include "format.h"
#include <cmdlime/errors.h>
#include <gsl/gsl>
#include <utility>
#include <vector>
#include <deque>
#include <unordered_set>
#include <algorithm>
#include <functional>

namespace cmdlime::detail{
using namespace gsl;

template <FormatType formatType>
class Parser{
protected:
    enum class ReadMode{
        All,
        Args,
        Commands,
        ExitFlagsAndCommands,
    };

private:
    using OutputFormatter = typename Format<formatType>::outputFormatter;

    class ReadModeScope{
    public:
        ReadModeScope(ReadMode value, ReadMode& readMode)
            : readMode_(readMode)
        {
            originalValue_ = readMode_;
            readMode_ = value;
        }
        ~ReadModeScope()
        {
            readMode_ = originalValue_;
        }
    private:
        ReadMode& readMode_;
        ReadMode originalValue_;
    };

public:
    Parser(std::vector<not_null<IParam*>> params,
           std::vector<not_null<IParamList*>> paramLists,
           std::vector<not_null<IFlag*>> flags,
           std::vector<not_null<IArg*>> args,
           IArgList* argList,
           std::vector<not_null<ICommand*>> commands)
        : params_(std::move(params))
        , paramLists_(std::move(paramLists))
        , flags_(std::move(flags))
        , args_(std::move(args))
        , argList_(argList)
        , commands_(std::move(commands))
    {}
    virtual ~Parser() = default;

    void parse(const std::vector<std::string>& cmdLine)
    {
        checkNames();
        argsToRead_.clear();
        std::copy(args_.begin(), args_.end(), std::back_inserter(argsToRead_));

        if (readCommandsAndExitFlags(cmdLine))
            return;

        preProcess();
        auto argsDelimiterEncountered = false;
        for (auto i = 0u; i < cmdLine.size(); ++i){
            const auto& token = cmdLine.at(i);
            if (token == "--"){
                argsDelimiterEncountered = true;
                continue;
            }
            if (!argsDelimiterEncountered){
                process(token);
                if (foundCommand_){
                    readCommand(foundCommand_, {cmdLine.begin() + i + 1, cmdLine.end()});
                    break;
                }
            }
            else{
                auto modeGuard = setScopeReadMode(ReadMode::Args);
                readArg(token);
            }
        }
        postProcess();

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
                default:
                    return false;
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
                default:
                    return false;
                }
            });
        if (paramListIt == paramLists_.end())
            return nullptr;
        return *paramListIt;
    }

    void readParam(const std::string& name, const std::string& value)
    {
        if (readMode_ != ReadMode::All)
            return;

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
                throw ParsingError{"Couldn't set parameter '" + OutputFormatter::paramPrefix() + paramList->info().name() + "' value from '" + value + "'"};
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
                default:
                    return false;
                }
            });
        if (flagIt == flags_.end())
            return nullptr;
        return *flagIt;
    }

    void readFlag(const std::string& name)
    {
        if (readMode_ != ReadMode::ExitFlagsAndCommands &&
            readMode_ != ReadMode::All)
            return;

        auto flag = findFlag(name);
        if (!flag)
            throw ParsingError{"Encountered unknown flag '" + OutputFormatter::flagPrefix() + name + "'"};
        flag->set();
    }

    void readArg(const std::string& value)
    {        
        if (readMode_ == ReadMode::All ||
            readMode_ == ReadMode::ExitFlagsAndCommands ||
            readMode_ == ReadMode::Commands){
            foundCommand_ = findCommand(value);
            if (foundCommand_)
                return;
        }
        if (readMode_ != ReadMode::Args &&
            readMode_ != ReadMode::All)
            return;

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


    void forEachParamInfo(const std::function<void(const OptionInfo&)>& handler)
    {
        for (auto param : params_)
            handler(param->info());
    }

    void forEachParamListInfo(const std::function<void(const OptionInfo&)>& handler)
    {
        for (auto paramList : paramLists_)
            handler(paramList->info());
    }

    void forEachFlagInfo(const std::function<void(const OptionInfo&)>& handler)
    {
        for (auto flag : flags_)
            handler(flag->info());
    }

private:
    virtual void preProcess(){}
    virtual void process(const std::string& cmdLineToken) = 0;
    virtual void postProcess(){}

    ICommand* findCommand(const std::string& name)
    {
        auto commandIt = std::find_if(commands_.begin(), commands_.end(),
            [&](auto command){
                return command->info().name() == name;

            });
        if (commandIt == commands_.end())
            return nullptr;
        return *commandIt;
    }

    void readCommand(ICommand* command, const std::vector<std::string>& cmdLine)
    {
        try{
            command->read(cmdLine);
        }
        catch(const ConfigError& error){
            throw CommandConfigError(command->info().name(), command->usageInfo(), error);
        }
        catch(const ParsingError& error){
            throw CommandParsingError(command->info().name(), command->usageInfo(), error);
        }
    }

    bool readCommandsAndExitFlags(const std::vector<std::string>& cmdLine)
    {
        auto modeGuard = setScopeReadMode(ReadMode::ExitFlagsAndCommands);

        preProcess();
        for (auto i = 0u; i < cmdLine.size(); ++i){
            const auto& token = cmdLine.at(i);
            if (token == "--")
                return false;

            process(token);
            if (foundCommand_){
                if (foundCommand_->isSubCommand()){
                    foundCommand_ = nullptr;
                    return false;
                }
                else{
                    readCommand(foundCommand_, {cmdLine.begin() + i + 1, cmdLine.end()});
                    foundCommand_ = nullptr;
                    return true;
                }
            }
        }
        if (isExitFlagSet())
            return true;

        return false;
    }

    ReadModeScope setScopeReadMode(ReadMode value)
    {
        return ReadModeScope{value, readMode_};
    }

    bool isExitFlagSet()
    {
        for (const auto flag : flags_)
            if (flag->isSet() && flag->isExitFlag())
                return true;
        return false;
    }

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

        auto processName = [&encounteredNames](const std::string& varType, const OptionInfo& var){
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
    ReadMode readMode_ = ReadMode::All;

private:
    std::vector<not_null<IParam*>> params_;
    std::vector<not_null<IParamList*>> paramLists_;
    std::vector<not_null<IFlag*>> flags_;
    std::vector<not_null<IArg*>> args_;    
    IArgList* argList_;
    std::vector<not_null<ICommand*>> commands_;
    std::deque<not_null<IArg*>> argsToRead_;
    ICommand* foundCommand_ = nullptr;
};


}
