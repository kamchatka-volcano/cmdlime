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
class X11Parser : public Parser<formatType>
{
    using Parser<formatType>::Parser;
    void process(const std::vector<std::string>& cmdLine) override
    {
        checkNames();
        auto foundParam = std::string{};
        for (const auto& part : cmdLine)
        {            
            if (str::startsWith(part, "-")){
                auto command = str::after(part, "-");
                if (isParamOrFlag(command) && !foundParam.empty())
                    throw ParsingError{"Parameter '-" + foundParam + "' value can't be empty"};

                if (auto param = this->findParam(command))
                    foundParam = param->info().name();
                else if (auto paramList = this->findParamList(command))
                    foundParam = paramList->info().name();
                else if (this->findFlag(command))
                    this->readFlag(command);
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
        auto check = [](ConfigVar& var, const std::string& varType){
            if (var.name().empty())
                throw ConfigError{varType + "'s name can't be empty"};
        };
        for (auto param : this->params_)
            check(param->info(), "Parameter");
        for (auto paramList : this->paramLists_)
            check(paramList->info(), "Parameter");
        for (auto flag : this->flags_)
            check(flag->info(), "Flag");
    }
};

class X11NameProvider{
public:
    static std::string name(const std::string& configVarName)
    {
        return toLowerCase(configVarName);
    }

    static std::string shortName(const std::string&)
    {
        return {};
    }

    static std::string valueName(const std::string& typeName)
    {
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
struct Format<FormatType::X11>
{
    using parser = X11Parser<FormatType::X11>;
    using nameProvider = X11NameProvider;
    using outputFormatter = X11OutputFormatter;
    static constexpr bool shortNamesEnabled = true;
};


}
