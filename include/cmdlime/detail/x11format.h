#pragma once
#include "parser.h"
#include "format.h"
#include "nameutils.h"
#include "utils.h"
#include <sfun/string_utils.h>
#include <cmdlime/errors.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>
#include <cassert>

namespace cmdlime::detail{
namespace str = sfun::string_utils;

template <FormatType formatType>
class X11Parser : public Parser<formatType>
{
    using Parser<formatType>::Parser;

    void preProcess() override
    {
        checkNames();
        foundParam_.clear();
    }

    void process(const std::string& token) override
    {        
        if (str::startsWith(token, "-") && token.size() > 1){
            auto command = str::after(token, "-");
            if (isParamOrFlag(command) && !foundParam_.empty())
                throw ParsingError{"Parameter '-" + foundParam_ + "' value can't be empty"};

            if (auto param = this->findParam(command))
                foundParam_ = param->info().name();
            else if (auto paramList = this->findParamList(command))
                foundParam_ = paramList->info().name();
            else if (this->findFlag(command))
                this->readFlag(command);
            else if (isNumber(token))
                this->readArg(token);
            else
                throw ParsingError{"Encountered unknown parameter or flag '" + token + "'"};
        }
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
            throw ParsingError{"Parameter '-" + foundParam_ + "' value can't be empty"};
    }

    bool isParamOrFlag(const std::string& cmd)
    {
        if (cmd.empty())
            return false;
        return this->findFlag(cmd) ||
               this->findParam(cmd) ||
               this->findParamList(cmd);
    }

    void checkNames()
    {
        auto check = [](const OptionInfo& var, const std::string& varType){
            if (!std::isalpha(var.name().front()))
                throw ConfigError{varType + "'s name '" + var.name() + "' must start with an alphabet character"};
            if (var.name().size() > 1){
                auto nonSupportedCharIt = std::find_if(var.name().begin() + 1, var.name().end(), [](char ch){return !std::isalnum(ch) && ch != '-';});
                if (nonSupportedCharIt != var.name().end())
                    throw ConfigError{varType + "'s name '" + var.name() + "' must consist of alphanumeric characters and hyphens"};
            }
        };
        this->forEachParamInfo([check](const OptionInfo& var){
            check(var, "Parameter");
        });
        this->forEachParamListInfo([check](const OptionInfo& var){
            check(var, "Parameter");
        });
        this->forEachFlagInfo([check](const OptionInfo& var){
            check(var, "Flag");
        });
    }

private:
    std::string foundParam_;
};

class X11NameProvider{
public:
    static std::string name(const std::string& optionName)
    {
        Expects(!optionName.empty());
        return toLowerCase(optionName);
    }

    static std::string shortName(const std::string& optionName)
    {
        Expects(!optionName.empty());
        return {};
    }

    static std::string fullName(const std::string& optionName)
    {
        Expects(!optionName.empty());
        return toLowerCase(optionName);
    }

    static std::string valueName(const std::string& typeName)
    {
        Expects(!typeName.empty());
        return toLowerCase(templateType(typeNameWithoutNamespace(typeName)));
    }
};


class X11OutputFormatter{
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
struct Format<FormatType::X11>
{
    using parser = X11Parser<FormatType::X11>;
    using nameProvider = X11NameProvider;
    using outputFormatter = X11OutputFormatter;
    static constexpr bool shortNamesEnabled = true;
};


}
