#pragma once
#include "iparam.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "configvar.h"
#include "format.h"
#include "string_utils.h"
#include "usageinfoformat.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>
#include <iomanip>


namespace cmdlime::detail{

inline std::string adjustedToLineBreak(std::string line, std::string& text)
{
    if (!text.empty() && !isspace(text.front())){
        auto trimmedLine = str::trimmedFront(line);
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
    assert(width >= 0);
    auto newLinePos = text.find('\n');
    if (newLinePos != std::string::npos && newLinePos <= width){
        auto line = text.substr(0, newLinePos);
        text.erase(text.begin(), text.begin() + static_cast<int>(newLinePos + 1));
        return line;
    }

    auto line = text.substr(0, width);
    if (text.size() < width)
        text.clear();
    else
        text.erase(text.begin(), text.begin() + static_cast<int>(width));
    if (!firstLine)
        line = str::trimmedFront(line);
    return adjustedToLineBreak(line, text);
}

inline std::vector<IParam*> filterParams(const std::vector<IParam*>& params, bool isOptional)
{
    auto result = std::vector<IParam*>{};
    std::copy_if(params.begin(), params.end(), std::back_inserter(result),
                 [isOptional](IParam* param){return param->isOptional() == isOptional;});
    return result;
}

template <typename T>
const std::string getName(T* configVar)
{
    return configVar->info().name();
}

template <typename T>
const std::string getType(T* configVar)
{
    return configVar->info().type();
}

template <typename T>
const std::string getDescription(T* configVar)
{
    return configVar->info().description();
}

template <FormatType formatType>
class UsageInfoCreator{
public:
    UsageInfoCreator(const std::string& programName,
                     UsageInfoFormat outputSettings,
                     std::vector<IParam*> params,
                     std::vector<IFlag*> flags,
                     std::vector<IArg*> args,
                     IArgList* argList)
    : programName_(programName)
    , params_(filterParams(params, false))
    , optionalParams_(filterParams(params, true))
    , flags_(flags)
    , args_(args)
    , argList_(argList)
    , outputSettings_(outputSettings)
    , maxOptionNameSize_(maxOptionNameSize() + outputSettings.columnsSpacing)
    {
    }

    std::string createDetailed()
    {
        return minimizedUsageInfo() +
               argsInfo() +
               paramsInfo() +
               optionsInfo() +
               flagsInfo();
    }

    std::string create()
    {
        return usageInfo();
    }

private:
    using OutputFormatter = typename Format<formatType>::outputFormatter;

    std::string usageInfo()
    {
        auto result = "Usage: " + programName_ + " ";
        for (auto arg : args_)
            result += OutputFormatter::argUsageName(*arg) + " ";

        for (auto param : params_)
            result += OutputFormatter::paramUsageName(*param) + " ";
        for (auto param : optionalParams_)
           result += "[" + OutputFormatter::paramUsageName(*param) + "] ";

        for (auto flag : flags_)
            result += "[" + OutputFormatter::flagUsageName(*flag) + "] ";

        if (argList_)
            result += OutputFormatter::argListUsageName(*argList_) + "\n";

        return result;
    }

    std::string minimizedUsageInfo()
    {
        auto result = "Usage: " + programName_ + " ";
        for (auto arg : args_)
            result += OutputFormatter::argUsageName(*arg) + " ";

        for (auto param : params_)
            result += OutputFormatter::paramUsageName(*param) + " ";

        if (!optionalParams_.empty())
            result += "[params] ";

        if (!flags_.empty())
            result += "[flags] ";

        if (argList_)
            result += OutputFormatter::argListUsageName(*argList_) + "\n";

        return result;
    }

    std::string paramsInfo()
    {
        auto result = std::string{"Parameters:\n"};
        if (params_.empty())
            return result;
        for (const auto& param : params_)
            result += makeConfigFieldInfo(OutputFormatter::paramDescriptionName(*param, outputSettings_.nameIndentation) + "\n", getDescription(param));
        return result;
    }

    std::string optionsInfo()
    {
        if (optionalParams_.empty())
            return {};
        auto result = std::string{};
        for (const auto& option : optionalParams_){
            auto defaultValueInfo = std::string{};
            if (!option->defaultValue().empty())
                defaultValueInfo = "\nDefault: " + option->defaultValue();
            result += makeConfigFieldInfo(OutputFormatter::paramDescriptionName(*option, outputSettings_.nameIndentation) + "\n", getDescription(option) + defaultValueInfo);
        }
        return result;
    }

    std::string argsInfo()
    {
        if (args_.empty() && !argList_)
            return {};
        auto result = std::string{"Arguments:\n"};
        for (const auto& arg : args_)
            result += makeConfigFieldInfo(OutputFormatter::argDescriptionName(*arg, outputSettings_.nameIndentation) + "\n", getDescription(arg));

        if (argList_)
            result += makeConfigFieldInfo(OutputFormatter::argListDescriptionName(*argList_, outputSettings_.nameIndentation) + "\n", getDescription(argList_));
        return result;
    }

    std::string flagsInfo()
    {
        if (flags_.empty())
            return {};
        auto result = std::string{"Flags:\n"};
        for (const auto& flag : flags_)
            result += makeConfigFieldInfo(OutputFormatter::flagDescriptionName(*flag, outputSettings_.nameIndentation) + "\n", getDescription(flag));
        return result;
    }

    int maxOptionNameSize()
    {        
        auto size = std::size_t{0};
        auto updateSize = [&size](std::string name)
        {
            auto firstLine = true;
            do {
               auto nameLine = popLine(name, 100, firstLine);
               size = std::max(size, nameLine.size());
               firstLine = false;
            } while(!name.empty());
        };

        for (auto param : params_)
            updateSize(OutputFormatter::paramDescriptionName(*param, outputSettings_.nameIndentation));
        for (auto option : optionalParams_)
            updateSize(OutputFormatter::paramDescriptionName(*option, outputSettings_.nameIndentation));
        for (auto flag : flags_)
            updateSize(OutputFormatter::flagDescriptionName(*flag, outputSettings_.nameIndentation));
        for (auto arg : args_)
            updateSize(OutputFormatter::argDescriptionName(*arg, outputSettings_.nameIndentation));
        if (argList_)
            updateSize(OutputFormatter::argListDescriptionName(*argList_, outputSettings_.nameIndentation));
        return size;
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
            auto nameLine = popLine(name, maxNameWidth, firstLine);
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
    std::string programName_;    
    std::vector<IParam*> params_;
    std::vector<IParam*> optionalParams_;
    std::vector<IFlag*> flags_;
    std::vector<IArg*> args_;
    IArgList* argList_;    
    UsageInfoFormat outputSettings_;
    int maxOptionNameSize_;
};

}
