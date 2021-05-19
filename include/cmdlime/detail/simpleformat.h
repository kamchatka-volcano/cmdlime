#pragma once
#include "parser.h"
#include "format.h"
#include "string_utils.h"
#include "nameutils.h"
#include "utils.h"
#include <cmdlime/errors.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>

namespace cmdlime::detail{

template <FormatType formatType>
class DefaultParser : public Parser<formatType>
{
    using Parser<formatType>::Parser;

    void process(const std::vector<std::string>& cmdLine) override
    {
        checkNames();
        for (const auto& part : cmdLine)
        {
            if (str::startsWith(part, "--") && part.size() > 2){
                const auto flagName = str::after(part, "--");                
                this->readFlag(flagName);
            }
            else if (str::startsWith(part, "-") && part.size() > 1){
                if (isNumber(part)){
                    this->readArg(part);
                    continue;
                }

                if (part.find('=') == std::string::npos)
                    throw ParsingError{"Wrong parameter format: " + part + ". Parameter must have a form of -name=value"};

                const auto paramName = str::before(str::after(part, "-"), "=");
                const auto paramValue = str::after(part, "=");                
                this->readParam(paramName, paramValue);
            }
            else
                this->readArg(part);
        }
    }

    void checkNames()
    {
        auto check = [](ConfigVar& var, const std::string& varType){
            if (!std::isalpha(var.name().front()))
                throw ConfigError{varType + "'s name '" + var.name() + "' must start with an alphabet character"};
            if (var.name().size() > 1){
                auto nonAlphaNumCharIt = std::find_if(var.name().begin() + 1, var.name().end(), [](char ch){return !std::isalnum(ch);});
                if (nonAlphaNumCharIt != var.name().end())
                    throw ConfigError{varType + "'s name '" + var.name() + "' must consist of alphanumeric characters"};
            }
        };
        for (auto param : this->params_)
            check(param->info(), "Parameter");
        for (auto paramList : this->paramLists_)
            check(paramList->info(), "Parameter");
        for (auto flag : this->flags_)
            check(flag->info(), "Flag");
    }
};

class DefaultNameProvider{
public:
    static std::string name(const std::string& configVarName)
    {
        Expects(!configVarName.empty());
        return toCamelCase(configVarName);
    }

    static std::string shortName(const std::string& configVarName)
    {
        Expects(!configVarName.empty());
        return {};
    }

    static std::string argName(const std::string& configVarName)
    {
        Expects(!configVarName.empty());
        return toCamelCase(configVarName);
    }

    static std::string valueName(const std::string& typeName)
    {
        Expects(!typeName.empty());
        return toCamelCase(templateType(typeNameWithoutNamespace(typeName)));
    }

};


class DefaultOutputFormatter{
public:
    static std::string paramUsageName(const IParam& param)
    {
        auto stream = std::stringstream{};
        if (param.isOptional())
            stream << "[" << paramPrefix() << param.info().name() << "=<" << param.info().valueName() << ">]";
        else
            stream << paramPrefix() << param.info().name() << "=<" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListUsageName(const IParamList& param)
    {
        auto stream = std::stringstream{};
        if (param.isOptional())
            stream << "[" << paramPrefix() << param.info().name() << "=<" << param.info().valueName() << ">...]";
        else
            stream << paramPrefix() << param.info().name() << "=<" << param.info().valueName() << ">...";
        return stream.str();
    }

    static std::string paramDescriptionName(const IParam& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << paramPrefix()
               << param.info().name() << "=<" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListDescriptionName(const IParamList& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << paramPrefix()
               << param.info().name() << "=<" << param.info().valueName() << ">";
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
struct Format<FormatType::Simple>
{
    using parser = DefaultParser<FormatType::Simple>;
    using nameProvider = DefaultNameProvider;
    using outputFormatter = DefaultOutputFormatter;
    static constexpr bool shortNamesEnabled = false;
};


}
