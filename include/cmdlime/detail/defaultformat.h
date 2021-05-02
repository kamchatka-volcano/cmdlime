#pragma once
#include "parser.h"
#include "format.h"
#include "string_utils.h"
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
        //2do: check '--' separator, check for negative digits
        for (auto it = cmdLine.begin(); it != cmdLine.end(); ++it)
        {
            if (str::startsWith(*it, "--")){
                const auto flagName = str::after(*it, "--");
                this->readFlag(flagName);
            }
            else if (str::startsWith(*it, "-")){
                const auto paramName = str::before(str::after(*it, "-"), "=");
                const auto paramValue = str::after(*it, "=");
                this->readParam(paramName, paramValue);
            }
            else
                this->readArg(*it);
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
