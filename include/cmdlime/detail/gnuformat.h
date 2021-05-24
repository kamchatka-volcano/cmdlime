#pragma once
#include "parser.h"
#include "format.h"
#include "string_utils.h"
#include "nameutils.h"
#include "utils.h"
#include "gsl/assert"
#include <cmdlime/errors.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>
#include <optional>

namespace cmdlime::detail{

template <FormatType formatType>
class GNUParser : public Parser<formatType>
{
    using Parser<formatType>::Parser;
    using FindMode = typename Parser<formatType>::FindMode;

    void processCommand(std::string command, std::string& foundParam, std::string& foundParamPrefix)
    {
        command = str::after(command, "--");
        auto paramValue = std::optional<std::string>{};
        if (command.find('=') != std::string::npos){
            paramValue = str::after(command, "=");
            command = str::before(command, "=");
        }
        if (isParamOrFlag(command) && !foundParam.empty())
            throw ParsingError{"Parameter '" + foundParamPrefix + foundParam + "' value can't be empty"};
        if (this->findParam(command, FindMode::Name) || this->findParamList(command, FindMode::Name)){
            if (paramValue.has_value())
                this->readParam(command, paramValue.value());
            else{
                foundParam = command;
                foundParamPrefix = "--";
            }
        }
        else if (this->findFlag(command, FindMode::Name))
            this->readFlag(command);
        else
            throw ParsingError{"Encountered unknown parameter or flag '--" + command + "'"};
    }

    void processShortCommand(std::string command, std::string& foundParam, std::string& foundParamPrefix)
    {
        auto possibleNumberArg = command;
        command = str::after(command, "-");
        if (isShortParamOrFlag(command)){
            if (!foundParam.empty())
                throw ParsingError{"Parameter '" + foundParamPrefix + foundParam + "' value can't be empty"};
            parseShortCommand(command, foundParam, foundParamPrefix);
        }
        else if (isNumber(possibleNumberArg))
            this->readArg(possibleNumberArg);
        else
            throw ParsingError{"Encountered unknown parameter or flag '-" + command + "'"};
    }

    void preProcess() override
    {
        checkNames();
        foundParam_.clear();
        foundParamPrefix_.clear();
    }

    void process(const std::string& token) override
    {        
        if (str::startsWith(token, "--") && token.size() > 2)
            processCommand(token, foundParam_, foundParamPrefix_);
        else if (str::startsWith(token, "-") && token.size() > 1)
            processShortCommand(token, foundParam_, foundParamPrefix_);
        else if (!foundParam_.empty()){
            this->readParam(foundParam_, token);
            foundParam_.clear();
        }
        else
            this->readArg(token);
    }

    void postProcess() override
    {
        if (!foundParam_.empty())
            throw ParsingError{"Parameter '" + foundParamPrefix_ + foundParam_ + "' value can't be empty"};
    }

    void parseShortCommand(const std::string& command, std::string& foundParam, std::string& foundParamPrefix)
    {
        if (command.empty())
            throw ParsingError{"Flags and parameters must have a name"};
        auto paramValue = std::string{};
        for(auto ch : command){
            auto opt = std::string{ch};
            if (!foundParam.empty())
                paramValue += opt;
            else if (this->findFlag(opt, FindMode::ShortName))
                this->readFlag(opt);
            else if (this->findParam(opt, FindMode::ShortName)){
                foundParam = opt;
                foundParamPrefix = "-";
            }
            else if (this->findParamList(opt, FindMode::ShortName)){
                foundParam = opt;
                foundParamPrefix = "-";
            }
            else
                throw ParsingError{"Unknown option '" + opt + "' in command '-" + command + "'"};
        }
        if (!foundParam.empty() && !paramValue.empty()){
            this->readParam(foundParam, paramValue);
            foundParam.clear();
        }
    }

    void checkLongNames()
    {
        auto checkName = [](ConfigVar& var, const std::string& varType){
            if (!std::isalpha(var.name().front()))
                throw ConfigError{varType + "'s name '" + var.name() + "' must start with an alphabet character"};
            if (var.name().size() > 1){
                auto nonSupportedCharIt = std::find_if(var.name().begin() + 1, var.name().end(), [](char ch){return !std::isalnum(ch) && ch != '-';});
                if (nonSupportedCharIt != var.name().end())
                    throw ConfigError{varType + "'s name '" + var.name() + "' must consist of alphanumeric characters and hyphens"};
            }
        };
        for (auto param : this->params_)
            checkName(param->info(), "Parameter");
        for (auto paramList : this->paramLists_)
            checkName(paramList->info(), "Parameter");
        for (auto flag : this->flags_)
            checkName(flag->info(), "Flag");
    }

    void checkShortNames()
    {
        auto checkShortName = [](ConfigVar& var, const std::string& varType){
            if (var.shortName().empty())
                return;
            if (var.shortName().size() != 1)
                throw ConfigError{varType + "'s short name '" + var.shortName() + "' can't have more than one symbol"};
            if (!std::isalnum(var.shortName().front()))
                throw ConfigError{varType + "'s short name '" + var.shortName() + "' must be an alphanumeric character"};
        };
        for (auto param : this->params_)
            checkShortName(param->info(), "Parameter");
        for (auto paramList : this->paramLists_)
            checkShortName(paramList->info(), "Parameter");
        for (auto flag : this->flags_)
            checkShortName(flag->info(), "Flag");
    }

    void checkNames()
    {
        checkLongNames();
        checkShortNames();
    }

    bool isParamOrFlag(const std::string& str)
    {
        if (str.empty())
            return false;
        return this->findFlag(str, FindMode::Name) ||
               this->findParam(str, FindMode::Name) ||
               this->findParamList(str, FindMode::Name);
    }

    bool isShortParamOrFlag(const std::string& str)
    {
        if (str.empty())
            return false;
        auto opt = str.substr(0,1);
        return this->findFlag(opt, FindMode::ShortName) ||
               this->findParam(opt, FindMode::ShortName) ||
               this->findParamList(opt, FindMode::ShortName);
    }

private:
    std::string foundParam_;
    std::string foundParamPrefix_;
};

class GNUNameProvider{
public:
    static std::string name(const std::string& configVarName)
    {
        Expects(!configVarName.empty());
        return toKebabCase(configVarName);
    }

    static std::string shortName(const std::string& configVarName)
    {
        Expects(!configVarName.empty());
        return toLowerCase(configVarName.substr(0,1));
    }

    static std::string argName(const std::string& configVarName)
    {
        Expects(!configVarName.empty());
        return toKebabCase(configVarName);
    }

    static std::string valueName(const std::string& typeName)
    {
        Expects(!typeName.empty());
        return toKebabCase(templateType(typeNameWithoutNamespace(typeName)));
    }
};


class GNUOutputFormatter{
public:
    static std::string paramUsageName(const IParam& param)
    {
        auto stream = std::stringstream{};
        if (param.isOptional())
            stream << "[" << paramPrefix() << param.info().name() << " <" << param.info().valueName() << ">]";
        else
            stream << paramPrefix() << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListUsageName(const IParamList& param)
    {
        auto stream = std::stringstream{};
        if (param.isOptional())
            stream << "[" << paramPrefix() << param.info().name() << " <" << param.info().valueName() << ">...]";
        else
            stream << paramPrefix() << param.info().name() << " <" << param.info().valueName() << ">...";
        return stream.str();
    }

    static std::string paramDescriptionName(const IParam& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent);
        if (!param.info().shortName().empty())
            stream << "-" << param.info().shortName() << ", ";
        else
            stream << " " << "   ";
        stream << "--" << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListDescriptionName(const IParamList& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent);
        if (!param.info().shortName().empty())
            stream << "-" << param.info().shortName() << ", ";
        else
            stream << " " << "   ";
        stream << "--" << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramPrefix()
    {
        return "--";
    }

    static std::string flagUsageName(const IFlag& flag)
    {
        auto stream = std::stringstream{};
        stream << "[" << flagPrefix() << flag.info().name() << "]";
        return stream.str();
    }

    static std::string flagDescriptionName(const IFlag& flag, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) ;
        if (!flag.info().shortName().empty())
            stream << "-" << flag.info().shortName() << ", ";
        else
            stream << " " << "   ";
        stream << "--" << flag.info().name();
        return stream.str();
    }

    static std::string flagPrefix()
    {
        return "--";
    }    

    static std::string argUsageName(const IArg& arg)
    {
        auto stream = std::stringstream{};
        stream << "<" << arg.info().name() << ">";
        return stream.str();
    }

    static std::string argDescriptionName(const IArg& arg, int indent = 0)
    {
        auto stream = std::stringstream{};
        if (indent)
            stream << std::setw(indent) << " ";
        stream << "<" << arg.info().name() << "> (" << arg.info().valueName() << ")";
        return stream.str();
    }

    static std::string argListUsageName(const IArgList& argList)
    {
        auto stream = std::stringstream{};
        if (argList.isOptional())
            stream << "[" << argList.info().name() << "...]";
        else
            stream << "<" << argList.info().name() << "...>";
        return stream.str();
    }

    static std::string argListDescriptionName(const IArgList& argList, int indent = 0)
    {
        auto stream = std::stringstream{};
        if (indent)
            stream << std::setw(indent) << " ";
        stream << "<" << argList.info().name() << "> (" << argList.info().valueName() << ")";
        return stream.str();
    }

};

template<>
struct Format<FormatType::GNU>
{
    using parser = GNUParser<FormatType::GNU>;
    using nameProvider = GNUNameProvider;
    using outputFormatter = GNUOutputFormatter;
    static constexpr bool shortNamesEnabled = true;
};


}
