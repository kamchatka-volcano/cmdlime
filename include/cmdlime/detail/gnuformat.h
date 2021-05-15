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
#include <optional>
#include <cassert>

namespace cmdlime::detail{



template <FormatType formatType>
class GNUParser : public Parser<formatType>
{
    using Parser<formatType>::Parser;
    using FindMode = typename Parser<formatType>::FindMode;
    void process(const std::vector<std::string>& cmdLine) override
    {
        checkNames();
        auto foundParam = std::string{};

        for (const auto& part : cmdLine){
            if (str::startsWith(part, "--")){
                auto command = str::after(part, "--");
                auto paramValue = std::optional<std::string>{};
                if (command.find('=') != std::string::npos){
                    paramValue = str::after(command, "=");
                    command = str::before(command, "=");
                }
                if (isParamOrFlag(command) && !foundParam.empty())
                    throw ParsingError{"Parameter '-" + foundParam + "' value can't be empty"};
                if (this->findParam(command, FindMode::Name) || this->findParamList(command, FindMode::Name)){
                    if (paramValue.has_value())
                        this->readParam(command, paramValue.value());
                    else
                        foundParam = command;
                }
                else if (this->findFlag(command, FindMode::Name))
                    this->readFlag(command);
                else
                    throw ParsingError{"Encountered unknown parameter or flag '" + part + "'"};
            }
            else if (str::startsWith(part, "-")){
                auto command = str::after(part, "-");
                if (isShortParamOrFlag(command)){
                    if (!foundParam.empty())
                        throw ParsingError{"Parameter '-" + foundParam + "' value can't be empty"};
                    readCommand(command, foundParam);
                }
                else if (isNumber(part))
                    this->readArg(part);
                else
                    throw ParsingError{"Encountered unknown parameter or flag '" + part + "'"};
            }
            else if (!foundParam.empty()){
                this->readParam(foundParam, part);
                foundParam.clear();
            }
            else
                this->readArg(part);
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
            else if (this->findFlag(opt, FindMode::ShortName))
                this->readFlag(opt);
            else if (this->findParam(opt, FindMode::ShortName))
                foundParam = opt;
            else if (this->findParamList(opt, FindMode::ShortName))
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
        auto checkShortName = [](ConfigVar& var, const std::string& varType){
            if (var.shortName().empty())
                return;
            if (var.shortName().size() != 1)
                throw ConfigError{varType + "'s name can't have more than one symbol"};
            if (!std::isalnum(var.shortName().front()))
                throw ConfigError{"Parameter's name must be an alphanumeric character"};
        };
        for (auto param : this->params_)
            checkShortName(param->info(), "Parameter");
        for (auto paramList : this->paramLists_)
            checkShortName(paramList->info(), "Parameter");
        for (auto flag : this->flags_)
            checkShortName(flag->info(), "Flag");
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

};

class GNUNameProvider{
public:
    static std::string name(const std::string& configVarName)
    {
        assert(!configVarName.empty());
        return toKebabCase(configVarName);
    }

    static std::string shortName(const std::string&)
    {
        return {};
    }

    static std::string valueName(const std::string& typeName)
    {
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
        stream << "--" << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListDescriptionName(const IParamList& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent);
        if (!param.info().shortName().empty())
            stream << "-" << param.info().shortName() << ", ";
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
        stream << "--" << flag.info().name();
        return stream.str();
    }

    static std::string flagPrefix()
    {
        return "--";
    }

    static std::string argName(const IArg& arg)
    {
        return arg.info().name();
    }

    static std::string argUsageName(const IArg& arg)
    {
        auto stream = std::stringstream{};
        stream << "<" << argName(arg) << ">";
        return stream.str();
    }

    static std::string argDescriptionName(const IArg& arg, int indent = 0)
    {
        auto stream = std::stringstream{};
        if (indent)
            stream << std::setw(indent) << " ";
        stream << "<" << argName(arg) << "> (" << arg.info().valueName() << ")";
        return stream.str();
    }

    static std::string argListName(const IArgList& argList)
    {
        return argList.info().name();
    }

    static std::string argListUsageName(const IArgList& argList)
    {
        auto stream = std::stringstream{};
        if (argList.isOptional())
            stream << "[" << argListName(argList) << "...]";
        else
            stream << "<" << argListName(argList) << "...>";
        return stream.str();
    }

    static std::string argListDescriptionName(const IArgList& argList, int indent = 0)
    {
        auto stream = std::stringstream{};
        if (indent)
            stream << std::setw(indent) << " ";
        stream << "<" << argListName(argList) << "> (" << argList.info().valueName() << ")";
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