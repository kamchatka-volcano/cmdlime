#ifndef CMDLIME_POSIXFORMAT_H
#define CMDLIME_POSIXFORMAT_H

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
class PosixParser : public Parser<formatType> {
    using Parser<formatType>::Parser;

    void processCommand(std::string command)
    {
        sfun_precondition(sfun::starts_with(command, "-"));

        auto possibleNumberArg = command;
        command = sfun::after(command, "-").value();
        if (isParamOrFlag(command)) {
            if (this->readMode_ != Parser<formatType>::ReadMode::ExitFlagsAndCommands) {
                if (!foundParam_.empty())
                    throw ParsingError{"Parameter '-" + foundParam_ + "' value can't be empty"};
                if (argumentEncountered_)
                    throw ParsingError{"Flags and parameters must precede arguments"};
            }
            parseCommand(command);
        }
        else if (isNumber(possibleNumberArg)) {
            this->readArg(possibleNumberArg);
            argumentEncountered_ = true;
        }
        else if (this->readMode_ != Parser<formatType>::ReadMode::ExitFlagsAndCommands)
            throw ParsingError{"Encountered unknown parameter or flag '-" + command + "'"};
    }

    void preProcess() override
    {
        checkNames();
        argumentEncountered_ = false;
        foundParam_.clear();
    }

    void process(const std::string& token) override
    {
        if (!foundParam_.empty()) {
            this->readParam(foundParam_, token);
            foundParam_.clear();
        }
        else if (sfun::starts_with(token, "-") && token.size() > 1)
            processCommand(token);
        else {
            this->readArg(token);
            argumentEncountered_ = true;
        }
    }

    void postProcess() override
    {
        if (!foundParam_.empty())
            throw ParsingError{"Parameter '-" + foundParam_ + "' value can't be empty"};
    }

    void parseCommand(const std::string& command)
    {
        if (command.empty())
            throw ParsingError{"Flags and parameters must have a name"};
        auto paramValue = std::string{};
        for (auto ch : command) {
            auto opt = std::string{ch};
            if (!foundParam_.empty())
                paramValue += opt;
            else if (this->findFlag(opt))
                this->readFlag(opt);
            else if (this->findParam(opt) || this->findParamList(opt))
                foundParam_ = opt;
            else if (this->readMode_ != Parser<formatType>::ReadMode::ExitFlagsAndCommands)
                throw ParsingError{"Unknown option '" + opt + "' in command '-" + command + "'"};
        }
        if (!foundParam_.empty() && !paramValue.empty()) {
            this->readParam(foundParam_, paramValue);
            foundParam_.clear();
        }
    }

    void checkNames()
    {
        auto check = [](const OptionInfo& var, const std::string& varType)
        {
            if (var.name().size() != 1)
                throw ConfigError{varType + "'s name '" + var.name() + "' can't have more than one symbol"};
            if (!std::isalnum(var.name().front()))
                throw ConfigError{varType + "'s name '" + var.name() + "' must be an alphanumeric character"};
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

    bool isParamOrFlag(const std::string& str)
    {
        if (str.empty())
            return false;
        auto opt = str.substr(0, 1);
        return this->findFlag(opt) || this->findParam(opt) || this->findParamList(opt);
    }

private:
    bool argumentEncountered_ = false;
    std::string foundParam_;
};

class PosixNameProvider {
public:
    static std::string name(sfun::not_empty<const std::string&> optionName)
    {
        return std::string{static_cast<char>(std::tolower(optionName.get().front()))};
    }

    static std::string shortName(sfun::not_empty<const std::string&>)
    {
        return {};
    }

    static std::string fullName(sfun::not_empty<const std::string&> optionName)
    {
        return toKebabCase(optionName);
    }

    static std::string valueName(sfun::not_empty<const std::string&> typeName)
    {
        return toCamelCase(templateType(typeNameWithoutNamespace(typeName)));
    }
};

class PosixOutputFormatter {
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
        stream << std::setw(indent) << paramPrefix() << param.info().name() << " <" << param.info().valueName() << ">";
        return stream.str();
    }

    static std::string paramListDescriptionName(const IParamList& param, int indent = 0)
    {
        auto stream = std::stringstream{};
        stream << std::setw(indent) << paramPrefix() << param.info().name() << " <" << param.info().valueName() << ">";
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
struct FormatCfg<Format::POSIX> {
    using parser = PosixParser<Format::POSIX>;
    using nameProvider = PosixNameProvider;
    using outputFormatter = PosixOutputFormatter;
    static constexpr bool shortNamesEnabled = false;
};

} //namespace cmdlime::detail

#endif //CMDLIME_POSIXFORMAT_H
