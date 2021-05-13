#pragma once
#include "parser.h"
#include "format.h"
#include "string_utils.h"
#include "nameutils.h"
#include "errors.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>
#include <cassert>

namespace cmdlime::detail{

inline bool isNameCorrect(const std::string& name)
{
    assert(!name.empty());
    if (!std::isalpha(name.front()))
        return false;
    if (name.size() == 1)
        return true;

    auto nonAlphaNumCharIt = std::find_if(name.begin() + 1, name.end(), [](char ch){return !std::isalnum(ch);});
    return nonAlphaNumCharIt == name.end();
}

template <FormatType formatType>
class DefaultParser : public Parser<formatType>
{
    using Parser<formatType>::Parser;
    void process(const std::vector<std::string>& cmdLine) override
    {                
        for (const auto& part : cmdLine)
        {
            if (str::startsWith(part, "--")){
                const auto flagName = str::after(part, "--");
                if (flagName.empty())
                    throw ConfigError{"Flag must have a name"};
                if (!isNameCorrect(flagName))
                    throw ConfigError{"Wrong flag name: '" + flagName + "'"};

                this->readFlag(flagName);
            }
            else if (str::startsWith(part, "-")){
                if (part.size() >= 2 && std::isdigit(part.at(1))){
                    this->readArg(part);
                    continue;
                }

                if (part.find('=') == std::string::npos)
                    throw ParsingError{"Wrong parameter format: " + part + ". Parameter must have a form of -name=value"};

                const auto paramName = str::before(str::after(part, "-"), "=");
                const auto paramValue = str::after(part, "=");
                if (!isNameCorrect(paramName))
                    throw ConfigError{"Wrong param name: '" + paramName + "'"};

                this->readParam(paramName, paramValue);
            }
            else
                this->readArg(part);
        }
    }
};

class DefaultNameProvider{
public:
    static std::string name(const std::string& configVarName)
    {
        return toCamelCase(configVarName);
    }

    static std::string shortName(const std::string&)
    {
        return {};
    }

    static std::string valueName(const std::string& typeName)
    {
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
struct Format<FormatType::Default>
{
    using parser = DefaultParser<FormatType::Default>;
    using nameProvider = DefaultNameProvider;
    using outputFormatter = DefaultOutputFormatter;
    static constexpr bool shortNamesEnabled = false;
};


}
