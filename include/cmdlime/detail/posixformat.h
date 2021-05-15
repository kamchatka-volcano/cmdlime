#pragma once
#include "parser.h"
#include "format.h"
#include "string_utils.h"
#include "nameutils.h"
#include "errors.h"
#include "utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>
#include <cassert>

namespace cmdlime::detail{

template <FormatType formatType>
class PosixParser : public Parser<formatType>
{
    using Parser<formatType>::Parser;

    void process(const std::vector<std::string>& cmdLine) override
    {
        checkNames();
        auto argumentEncountered = false;
        auto foundParam = std::string{};
        for (const auto& part : cmdLine){
            if (str::startsWith(part, "-")){
                auto command = str::after(part, "-");
                if (isParamOrFlag(command)){
                    if (!foundParam.empty())
                        throw ParsingError{"Parameter '-" + foundParam + "' value can't be empty"};
                    if (argumentEncountered)
                        throw ParsingError{"Flags and parameters must preceed arguments"};
                    readCommand(command, foundParam);
                }
                else if (isNumber(part)){
                    this->readArg(part);
                    argumentEncountered = true;
                }
                else
                    throw ParsingError{"Encountered unknown parameter or flag '" + part + "'"};
            }
            else if (!foundParam.empty()){
                this->readParam(foundParam, part);
                foundParam.clear();
            }
            else{
                this->readArg(part);
                argumentEncountered = true;
            }
        }
        if (!foundParam.empty())
            throw ParsingError{"Parameter '-" + foundParam + "' value can't be empty"};
    }

    void readCommand(const std::string& command, std::string& foundParam)
    {
        if (command.empty())
            throw ParsingError{"Flags and parameters must have a name"};
        auto paramValue = std::string{};
        for(auto ch : command){
            auto opt = std::string{ch};
            if (!foundParam.empty())
                paramValue += opt;
            else if (this->findFlag(opt))
                this->readFlag(opt);
            else if (this->findParam(opt))
                foundParam = opt;
            else if (this->findParamList(opt))
                foundParam = opt;
            else
                throw ParsingError{"Unknown option '" + opt + "' in command '-" + command + "'"};
        }
        if (!foundParam.empty() && !paramValue.empty()){
            this->readParam(foundParam, paramValue);
            foundParam.clear();
        }
    }

    void checkNames()
    {
        auto check = [](ConfigVar& var, const std::string& varType){
            if (var.name().size() != 1)
                throw ConfigError{varType + "'s name can't have more than one symbol"};
            if (!std::isalnum(var.name().front()))
                throw ConfigError{"Parameter's name must be an alphanumeric character"};
        };
        for (auto param : this->params_)
            check(param->info(), "Parameter");
        for (auto paramList : this->paramLists_)
            check(paramList->info(), "Parameter");
        for (auto flag : this->flags_)
            check(flag->info(), "Flag");
    }

    bool isParamOrFlag(const std::string& str)
    {
        if (str.empty())
            return false;
        auto opt = str.substr(0,1);
        return this->findFlag(opt) ||
               this->findParam(opt) ||
               this->findParamList(opt);
    }

};

class PosixNameProvider{
public:
    static std::string name(const std::string& configVarName)
    {
        assert(!configVarName.empty());
        return std::string{static_cast<char>(std::tolower(configVarName.front()))};
    }

    static std::string shortName(const std::string&)
    {
        return {};
    }

    static std::string argName(const std::string& configVarName)
    {
        return toKebabCase(configVarName);
    }

    static std::string valueName(const std::string& typeName)
    {
        return toCamelCase(templateType(typeNameWithoutNamespace(typeName)));
    }
};


class PosixOutputFormatter{
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
        stream << std::setw(indent) << paramPrefix()
               << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListDescriptionName(const IParamList& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << paramPrefix()
               << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramPrefix()
    {
        return "-";
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
        stream << std::setw(indent) << flagPrefix() << flag.info().name();
        return stream.str();
    }

    static std::string flagPrefix()
    {
        return "-";
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
struct Format<FormatType::POSIX>
{
    using parser = PosixParser<FormatType::POSIX>;
    using nameProvider = PosixNameProvider;
    using outputFormatter = PosixOutputFormatter;
    static constexpr bool shortNamesEnabled = false;
};


}
