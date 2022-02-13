#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "icommand.h"
#include "optioninfo.h"
#include "options.h"
#include "formatcfg.h"
#include "string_utils.h"
#include <cmdlime/usageinfoformat.h>
#include <utility>
#include <vector>
#include <memory>
#include <algorithm>
#include <iomanip>

namespace cmdlime::detail{
using namespace gsl;
namespace str = string_utils;

inline std::string adjustedToLineBreak(std::string line, std::string& text)
{
    if (!text.empty() && !isspace(text.front())){
        auto trimmedLine = str::trimFront(line);
        if (std::find_if(trimmedLine.begin(), trimmedLine.end(), [](auto ch){return std::isspace(ch);}) == trimmedLine.end())
            return line;
        while(!isspace(line.back())){
            text.insert(text.begin(), 1, line.back());
            line.pop_back();
        }
    }
    return line;
}

inline std::string popLine(std::string& text, std::size_t width, bool firstLine = false)
{
    auto newLinePos = text.find('\n');
    if (newLinePos != std::string::npos && newLinePos <= width){
        auto line = text.substr(0, newLinePos);
        text.erase(text.begin(), text.begin() + static_cast<int>(newLinePos + 1));
        if (!firstLine)
            line = str::trimFront(line);
        return line;
    }

    auto line = text.substr(0, width);
    if (text.size() < width)
        text.clear();
    else
        text.erase(text.begin(), text.begin() + static_cast<int>(width));
    if (!firstLine)
        line = str::trimFront(line);
    return adjustedToLineBreak(line, text);
}

template <typename T>
inline std::vector<std::reference_wrapper<T>> getParamsByOptionality(const std::vector<std::unique_ptr<T>>& params, bool isOptional)
{
    auto result = std::vector<std::reference_wrapper<T>>{};
    auto paramRefList = std::vector<std::reference_wrapper<T>>{};
    std::transform(params.begin(), params.end(), std::back_inserter(paramRefList),
                   [](auto& param) -> T& {return *param;});
    std::copy_if(paramRefList.begin(), paramRefList.end(), std::back_inserter(result),
                 [isOptional](auto& param){return param.get().isOptional() == isOptional;});
    return result;
}

template <typename T>
const std::string& getName(T& option)
{
    return option.info().name();
}

template <typename T>
const std::string& getType(T& option)
{
    return option.info().type();
}

template <typename T>
const std::string& getDescription(T& option)
{
    return option.info().description();
}

template <Format formatType>
class UsageInfoCreator{
public:
    UsageInfoCreator(std::string commandName,
                     UsageInfoFormat outputSettings,
                     const Options& options)
    : commandName_(std::move(commandName))
    , params_(getParamsByOptionality(options.params(), false))
    , optionalParams_(getParamsByOptionality(options.params(), true))
    , paramLists_(getParamsByOptionality(options.paramLists(), false))
    , optionalParamLists_(getParamsByOptionality(options.paramLists(), true))
    , options_(options)
    , outputSettings_(outputSettings)
    , maxOptionNameSize_(maxOptionNameLength() + outputSettings.columnsSpacing)
    {
    }

    std::string createDetailed()
    {
        return minimizedUsageInfo() +
               argsInfo() +
               paramsInfo() +               
               paramListsInfo() +
               optionsInfo() +
               optionalParamListsInfo() +
               flagsInfo() +
               commandsInfo();
    }

    std::string create()
    {
        return usageInfo();
    }

private:
    using OutputFormatter = typename FormatCfg<formatType>::outputFormatter;

    std::string usageInfo()
    {
        auto result = "Usage: " + commandName_ + " ";
        if (!options_.commands().empty())
            result += "[commands] ";

        for (auto& arg : options_.args())
            result += OutputFormatter::argUsageName(*arg) + " ";

        for (auto& param : params_)
            result += OutputFormatter::paramUsageName(param) + " ";
        for (auto& paramList : paramLists_)
            result += OutputFormatter::paramListUsageName(paramList) + " ";
        for (auto& param : optionalParams_)
           result += OutputFormatter::paramUsageName(param) + " ";
        for (auto& paramList : optionalParamLists_)
           result += OutputFormatter::paramListUsageName(paramList) + " ";

        for (auto& flag : options_.flags())
            result += OutputFormatter::flagUsageName(*flag) + " ";

        if (options_.argList())
            result += OutputFormatter::argListUsageName(*options_.argList());

        result += "\n";
        return result;
    }

    std::string minimizedUsageInfo()
    {
        auto result = "Usage: " + commandName_ + " ";

        if (!options_.commands().empty())
            result += "[commands] ";

        for (auto& arg : options_.args())
            result += OutputFormatter::argUsageName(*arg) + " ";

        for (auto& param : params_)
            result += OutputFormatter::paramUsageName(param) + " ";

        for (auto& paramList : paramLists_)
            result += OutputFormatter::paramListUsageName(paramList) + " ";

        if (!optionalParams_.empty() || !optionalParamLists_.empty())
            result += "[params] ";

        if (!options_.flags().empty())
            result += "[flags] ";

        if (options_.argList())
            result += OutputFormatter::argListUsageName(*options_.argList());

        result += "\n";
        return result;
    }

    std::string paramsInfo()
    {
        auto result = std::string{"Parameters:\n"};
        if (params_.empty())
            return result;
        for (const IParam& param : params_){
            const auto name = OutputFormatter::paramDescriptionName(param, outputSettings_.nameIndentation) + "\n";
            result += makeConfigFieldInfo(name, getDescription(param));
        }
        return result;
    }

    std::string paramListsInfo()
    {
        auto result = std::string{};
        if (paramLists_.empty())
            return result;
        for (const IParamList& paramList : paramLists_){
            const auto name = OutputFormatter::paramListDescriptionName(paramList, outputSettings_.nameIndentation) + "\n";
            auto description = getDescription(paramList);
            if (!description.empty())
                description += "\n(multi-value)";
            else
                description += "multi-value";
            result += makeConfigFieldInfo(name, description);
        }
        return result;
    }

    std::string optionsInfo()
    {
        if (optionalParams_.empty())
            return {};
        auto result = std::string{};
        for (const IParam& option : optionalParams_){
            auto description = getDescription(option);
            if (!description.empty()){
                if (!option.defaultValue().empty())
                    description += "\n(optional, default: " + option.defaultValue() + ")";
                else
                    description += "\n(optional)";
            }
            else{
                if (!option.defaultValue().empty())
                    description += "optional, default: " + option.defaultValue();
                else
                    description += "optional";
            }
            result += makeConfigFieldInfo(OutputFormatter::paramDescriptionName(option, outputSettings_.nameIndentation) + "\n", description);
        }
        return result;
    }

    std::string optionalParamListsInfo()
    {
        if (optionalParamLists_.empty())
            return {};
        auto result = std::string{};
        for (const IParamList& option : optionalParamLists_){
            auto description = getDescription(option);
            if (!description.empty()){
                description += "\n(multi-value, ";
                if (!option.defaultValue().empty())
                    description += "optional, default: " + option.defaultValue() + ")";
                else
                    description += "optional)";
            }
            else{
                description += "multi-value, ";
                if (!option.defaultValue().empty())
                    description += "optional, default: " + option.defaultValue();
                else
                    description += "optional";
            }
            result += makeConfigFieldInfo(OutputFormatter::paramListDescriptionName(option, outputSettings_.nameIndentation) + "\n", description);
        }
        return result;
    }

    std::string argsInfo()
    {
        if (options_.args().empty() && !options_.argList())
            return {};
        auto result = std::string{"Arguments:\n"};
        for (const auto& arg : options_.args())
            result += makeConfigFieldInfo(OutputFormatter::argDescriptionName(*arg, outputSettings_.nameIndentation) + "\n", getDescription(*arg));

        if (options_.argList()){
            auto description = getDescription(*options_.argList());
            if (!description.empty()){
                description += "\n(multi-value";
                if (options_.argList()->isOptional()){
                    if (!options_.argList()->defaultValue().empty())
                        description += ", optional, default: " + options_.argList()->defaultValue() + ")";
                    else
                        description +=  ", optional)";
                }
                else
                    description += ")";
            }
            else{
                description += "multi-value";
                if (options_.argList()->isOptional()){
                    if (!options_.argList()->defaultValue().empty())
                        description += ", optional, default: " + options_.argList()->defaultValue();
                    else
                        description +=  ", optional";
                }
            }
            result += makeConfigFieldInfo(OutputFormatter::argListDescriptionName(*options_.argList(), outputSettings_.nameIndentation) + "\n", description);
        }
        return result;
    }

    std::string flagsInfo()
    {
        if (options_.flags().empty())
            return {};
        auto result = std::string{"Flags:\n"};
        for (const auto& flag : options_.flags())
            result += makeConfigFieldInfo(OutputFormatter::flagDescriptionName(*flag, outputSettings_.nameIndentation) + "\n", getDescription(*flag));
        return result;
    }

    std::string commandsInfo()
    {
        if (options_.commands().empty())
            return {};
        auto result = std::string{"Commands:\n"};
        for (const auto& command : options_.commands()){
            auto nameStream = std::stringstream{};
            if (outputSettings_.nameIndentation)
                nameStream << std::setw(outputSettings_.nameIndentation) << " ";
            nameStream << getName(*command)
                       << " [options]";
            result += makeConfigFieldInfo(nameStream.str(), getDescription(*command));
        }
        return result;
    }

    int maxOptionNameLength()
    {        
        auto length = 0;
        auto updateLength = [&length](std::string name)
        {
            auto firstLine = true;
            do {
               auto nameLine = popLine(name, 100, firstLine);
               length = std::max(length, static_cast<int>(nameLine.size()));
               firstLine = false;
            } while(!name.empty());
        };

        for (auto& param : params_)
            updateLength(OutputFormatter::paramDescriptionName(param, outputSettings_.nameIndentation));
        for (auto& option : optionalParams_)
            updateLength(OutputFormatter::paramDescriptionName(option, outputSettings_.nameIndentation));
        for (auto& flag : options_.flags())
            updateLength(OutputFormatter::flagDescriptionName(*flag, outputSettings_.nameIndentation));
        for (auto& arg : options_.args())
            updateLength(OutputFormatter::argDescriptionName(*arg, outputSettings_.nameIndentation));
        if (options_.argList())
            updateLength(OutputFormatter::argListDescriptionName(*options_.argList(), outputSettings_.nameIndentation));
        return length;
    }

    std::string makeConfigFieldInfo(std::string name, std::string description)
    {
        auto maxNameWidth = std::min(outputSettings_.maxNameColumnWidth, maxOptionNameSize_);
        const auto columnSeparatorWidth = 1;
        const auto leftColumnWidth = columnSeparatorWidth + maxNameWidth;
        const auto rightColumnWidth = static_cast<std::size_t>(outputSettings_.terminalWidth - leftColumnWidth);
        const auto descriptionWidth = rightColumnWidth - 2;
        auto stream = std::stringstream{};
        auto firstLine = true;
        while (!name.empty()){
            auto nameLine = popLine(name, static_cast<std::size_t>(maxNameWidth), firstLine);
            const auto descriptionLine = popLine(description, descriptionWidth, firstLine);
            if (firstLine)
                stream << std::setw(maxNameWidth) << std::left << nameLine << " " << descriptionLine << std::endl;
            else
                stream << std::setw(maxNameWidth) << std::left << nameLine << "   " << descriptionLine << std::endl;
            firstLine = false;
        }
        while(!description.empty()){
            auto descriptionLine = popLine(description, descriptionWidth);
            stream << std::setw(leftColumnWidth) << " " << "  " << descriptionLine << std::endl;
        }
        return stream.str();
    }

private:    
    std::string commandName_;
    std::vector<std::reference_wrapper<IParam>> params_;
    std::vector<std::reference_wrapper<IParam>> optionalParams_;
    std::vector<std::reference_wrapper<IParamList>> paramLists_;
    std::vector<std::reference_wrapper<IParamList>> optionalParamLists_;
    const Options& options_;
    UsageInfoFormat outputSettings_;
    int maxOptionNameSize_;
};

}
