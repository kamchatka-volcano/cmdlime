#ifndef CMDLIME_SIMPLEFORMAT_H
#define CMDLIME_SIMPLEFORMAT_H

#include "formatcfg.h"
#include "nameutils.h"
#include "parser.h"
#include "utils.h"
#include "external/sfun/precondition.h"
#include "external/sfun/string_utils.h"
#include <cmdlime/errors.h>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>

namespace cmdlime::detail {

template<Format formatType>
class DefaultParser : public Parser<formatType> {
    using Parser<formatType>::Parser;

    void preProcess() override
    {
        checkNames();
    }

    void process(const std::string& token) override
    {
        if (sfun::starts_with(token, "--") && token.size() > 2) {
            const auto flagName = sfun::after(token, "--").value();
            this->readFlag(flagName);
        }
        else if (sfun::starts_with(token, "-") && token.size() > 1) {
            if (isNumber(token)) {
                this->readArg(token);
                return;
            }

            if (token.find('=') == std::string::npos)
                throw ParsingError{"Wrong parameter format: " + token + ". Parameter must have a form of -name=value"};

            const auto paramName = sfun::between(token, "-", "=").value();
            const auto paramValue = std::string{sfun::after(token, "=").value()};
            this->readParam(paramName, paramValue);
        }
        else
            this->readArg(token);
    }

    void checkNames()
    {
        auto check = [](const OptionInfo& var, const std::string& varType)
        {
            if (!std::isalpha(var.name().front()))
                throw ConfigError{varType + "'s name '" + var.name() + "' must start with an alphabet character"};
            if (var.name().size() > 1) {
                auto nonAlphaNumCharIt = std::find_if(
                        var.name().begin() + 1,
                        var.name().end(),
                        [](char ch)
                        {
                            return !std::isalnum(ch);
                        });
                if (nonAlphaNumCharIt != var.name().end())
                    throw ConfigError{varType + "'s name '" + var.name() + "' must consist of alphanumeric characters"};
            }
        };
        this->forEachParamInfo(
                [check](const OptionInfo& var)
                {
                    check(var, "Parameter");
                });
        this->forEachParamListInfo(
                [check](const OptionInfo& var)
                {
                    check(var, "Parameter");
                });
        this->forEachFlagInfo(
                [check](const OptionInfo& var)
                {
                    check(var, "Flag");
                });
    }
};

class DefaultNameProvider {
public:
    static std::string name(sfun::not_empty<const std::string&> optionName)
    {
        return toCamelCase(optionName);
    }

    static std::string shortName(sfun::not_empty<const std::string&>)
    {
        return {};
    }

    static std::string fullName(sfun::not_empty<const std::string&> optionName)
    {
        return toCamelCase(optionName);
    }

    static std::string valueName(sfun::not_empty<const std::string&> typeName)
    {
        return toCamelCase(templateType(typeNameWithoutNamespace(typeName)));
    }
};

class DefaultOutputFormatter {
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
        stream << std::setw(indent) << paramPrefix() << param.info().name() << "=<" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListDescriptionName(const IParamList& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << paramPrefix() << param.info().name() << "=<" << param.info().valueName() << ">";
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

    static std::string commandDescriptionName(const ICommand& command, int indent = 0)
    {
        auto stream = std::stringstream{};
        if (indent)
            stream << std::setw(indent) << " ";
        stream << command.info().name() << " [options]";
        return stream.str();
    }
};

template<>
struct FormatCfg<Format::Simple> {
    using parser = DefaultParser<Format::Simple>;
    using nameProvider = DefaultNameProvider;
    using outputFormatter = DefaultOutputFormatter;
    static constexpr bool shortNamesEnabled = false;
};

} //namespace cmdlime::detail

#endif //CMDLIME_SIMPLEFORMAT_H
