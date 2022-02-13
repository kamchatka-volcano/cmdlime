#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "icommand.h"
#include "options.h"
#include "formatcfg.h"
#include <cmdlime/errors.h>
#include <utility>
#include <vector>
#include <deque>
#include <unordered_set>
#include <algorithm>
#include <functional>

namespace cmdlime::detail{
using namespace gsl;

template <Format formatType>
class Parser{
protected:
    enum class ReadMode{
        All,
        Args,
        Commands,
        ExitFlagsAndCommands,
    };

private:
    using OutputFormatter = typename FormatCfg<formatType>::outputFormatter;

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
    explicit Parser(const Options& options)
        : options_(options)
    {}
    virtual ~Parser() = default;

    void parse(const std::vector<std::string>& cmdLine)
    {
        checkNames();
        argsToRead_.clear();

        std::transform(options_.args().begin(), options_.args().end(),
                       std::back_inserter(argsToRead_),
                       [](auto& arg) ->IArg& { return *arg; });

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
        auto paramIt = std::find_if(options_.params().begin(), options_.params().end(),
            [&](auto& param){
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
        if (paramIt == options_.params().end())
            return nullptr;
        return paramIt->get();
    }

    IParamList* findParamList(const std::string& name, FindMode mode = FindMode::All)
    {
        auto paramListIt = std::find_if(options_.paramLists().begin(), options_.paramLists().end(),
            [&](auto& paramList){
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
        if (paramListIt == options_.paramLists().end())
            return nullptr;
        return paramListIt->get();
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
        auto flagIt = std::find_if(options_.flags().begin(), options_.flags().end(),
            [&](auto& flag){
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
        if (flagIt == options_.flags().end())
            return nullptr;
        return flagIt->get();
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
            auto& arg = static_cast<IArg&>(argsToRead_.front());
            if (value.empty())
                throw ParsingError{"Arg '" + arg.info().name() + "' value can't be empty"};
            argsToRead_.pop_front();
            if (!arg.read(value))
                throw ParsingError{"Couldn't set argument '" + arg.info().name() + "' value from '" + value + "'"};
        }
        else if (options_.argList()){
            if (value.empty())
                throw ParsingError{"Arg list '" + options_.argList()->info().name() + "' element value can't be empty"};
            if (!options_.argList()->read(value))
                throw ParsingError{"Couldn't set argument list '" + options_.argList()->info().name() + "' element's value from '" + value + "'"};
        }
        else
            throw ParsingError("Encountered unknown positional argument '" + value + "'");
    }


    void forEachParamInfo(const std::function<void(const OptionInfo&)>& handler)
    {
        for (auto& param : options_.params())
            handler(param->info());
    }

    void forEachParamListInfo(const std::function<void(const OptionInfo&)>& handler)
    {
        for (auto& paramList : options_.paramLists())
            handler(paramList->info());
    }

    void forEachFlagInfo(const std::function<void(const OptionInfo&)>& handler)
    {
        for (auto& flag : options_.flags())
            handler(flag->info());
    }

private:
    virtual void preProcess(){}
    virtual void process(const std::string& cmdLineToken) = 0;
    virtual void postProcess(){}

    ICommand* findCommand(const std::string& name)
    {
        auto commandIt = std::find_if(options_.commands().begin(), options_.commands().end(),
            [&](auto& command){
                return command->info().name() == name;

            });
        if (commandIt == options_.commands().end())
            return nullptr;
        return commandIt->get();
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
        for (const auto& flag : options_.flags())
            if (flag->isSet() && flag->isExitFlag())
                return true;
        return false;
    }

    void checkUnreadParams()
    {
        for (const auto& param : options_.params())
            if (!param->hasValue())
                throw ParsingError{"Parameter '" + OutputFormatter::paramPrefix() + param->info().name() + "' is missing."};

        for (const auto& paramList : options_.paramLists())
            if (!paramList->hasValue())
                throw ParsingError{"Parameter '" + OutputFormatter::paramPrefix() + paramList->info().name() + "' is missing."};
    }

    void checkUnreadArgs()
    {
        if(!argsToRead_.empty())
            throw ParsingError{"Positional argument '" + argsToRead_.front().get().info().name() + "' is missing."};
    }

    void checkUnreadArgList()
    {
        if (options_.argList() && !options_.argList()->hasValue())
            throw ParsingError{"Arguments list '" + options_.argList()->info().name() + "' is missing."};
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
        for (auto& param : options_.params())
            processName("Parameter's", param->info());
        for (auto& paramList : options_.paramLists())
            processName("Parameter's", paramList->info());
        for (auto& flag : options_.flags())
            processName("Flag's", flag->info());
    }

protected:
    ReadMode readMode_ = ReadMode::All;

private:
    const Options& options_;
    std::deque<std::reference_wrapper<IArg>> argsToRead_;
    ICommand* foundCommand_ = nullptr;
};


}
