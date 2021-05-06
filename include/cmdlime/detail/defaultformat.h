#pragma once
#include "parser.h"
#include "format.h"
#include "string_utils.h"
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
        //2do: check '--' separator, check for negative digits        
        for (const auto& part : cmdLine)
        {
            if (str::startsWith(part, "--")){
                const auto flagName = str::after(part, "--");
                if (flagName.empty())
                    throw ParsingError{"Flag must have a name"};
                if (!isNameCorrect(flagName))
                    throw ParsingError{"Wrong flag name: '" + flagName + "'"};

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
                    throw ParsingError{"Wrong param name: '" + paramName + "'"};

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
        return configVarName;
    }

    static std::string shortName(const std::string&)
    {
        return {};
    }
};


class DefaultOutputFormatter{
public:
    static std::string paramUsageName(const IParam& param)
    {
        auto stream = std::stringstream{};
        stream << paramPrefix() << param.info().name() << "=<" << param.info().type() << ">";
        return stream.str();
    }

    static std::string paramDescriptionName(const IParam& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << paramPrefix()
               << param.info().name() << "=<" << param.info().type() << ">";
        return stream.str();
    }

    static std::string paramPrefix()
    {
        return "-";
    }

    static std::string flagUsageName(const IFlag& flag)
    {
        auto stream = std::stringstream{};
        stream << flagPrefix() << flag.info().name();
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
        stream << std::setw(indent) << " " << "<" << arg.info().name() << "> (" << arg.info().type() << ")";
        return stream.str();
    }

    static std::string argListUsageName(const IArgList& argList)
    {
        auto stream = std::stringstream{};
        stream << "[" << argList.info().name() << "...]";
        return stream.str();
    }

    static std::string argListDescriptionName(const IArgList& argList, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << " " << "<" << argList.info().name() << ">... (" << argList.info().type() << ")";
        return stream.str();
    }

};

template<>
struct Format<FormatType::Default>
{
    using parser = DefaultParser<FormatType::Default>;
    using nameProvider = DefaultNameProvider;
    using outputFormatter = DefaultOutputFormatter;

};


}
