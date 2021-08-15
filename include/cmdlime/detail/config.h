#pragma once
#include "iparam.h"
#include "iparamlist.h"
#include "iflag.h"
#include "iarg.h"
#include "iarglist.h"
#include "icommand.h"
#include "format.h"
#include "usageinfocreator.h"
#include "configaccess.h"
#include "gsl/pointers"
#include <cmdlime/usageinfoformat.h>
#include <cmdlime/errors.h>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <memory>
#include <algorithm>

namespace cmdlime::detail{
using namespace gsl;

template <typename T>
inline std::vector<not_null<T*>> getPtrList(const std::vector<std::unique_ptr<T>>& ownerList)
{
    auto result = std::vector<not_null<T*>>{};
    std::transform(ownerList.begin(), ownerList.end(), std::back_inserter(result),
                   [](auto& owner){return owner.get();});
    return result;
}

template<FormatType formatType>
class Config{
public:
    void readCommandLine(int argc, char** argv)
    {
        auto cmdLine = std::vector<std::string>(argv + 1, argv + argc);
        read(cmdLine);
    }

    void read(const std::vector<std::string>& cmdLine)
    {
        if (!configError_.empty())
            throw ConfigError{configError_};
        using ParserType = typename Format<formatType>::parser;
        auto parser = ParserType{getPtrList(params_),
                                 getPtrList(paramLists_),
                                 getPtrList(flags_),
                                 getPtrList(args_),
                                 argList_.get(),
                                 getPtrList(commands_)};
        parser.parse(cmdLine);
    }

    const std::string& versionInfo() const
    {
        return versionInfo_;
    }

    std::string usageInfo(const std::string& name) const
    {
        if (!customUsageInfo_.empty())
            return customUsageInfo_;

        return UsageInfoCreator<formatType>{name,
                                            UsageInfoFormat{},
                                            getPtrList(params_),
                                            getPtrList(paramLists_),
                                            getPtrList(flags_),
                                            getPtrList(args_),
                                            argList_.get(),
                                            getPtrList(commands_)}.create();
    }

    std::string usageInfoDetailed(const std::string& name, UsageInfoFormat outputSettings = {}) const
    {
        if (!customUsageInfoDetailed_.empty())
            return customUsageInfoDetailed_;

        return UsageInfoCreator<formatType>{name,
                                            outputSettings,
                                            getPtrList(params_),
                                            getPtrList(paramLists_),
                                            getPtrList(flags_),
                                            getPtrList(args_),
                                            argList_.get(),
                                            getPtrList(commands_)}.createDetailed();
    }

    void setVersionInfo(const std::string& info)
    {
        versionInfo_ = info;
    }

    void setUsageInfo(const std::string& info)
    {
        customUsageInfo_ = info;
    }

    void setUsageInfoDetailed(const std::string& info)
    {
        customUsageInfoDetailed_ = info;
    }

private:
    void addParam(std::unique_ptr<IParam> param)
    {
        params_.emplace_back(std::move(param));
    }

    void addParamList(std::unique_ptr<IParamList> paramList)
    {
        paramLists_.emplace_back(std::move(paramList));
    }

    void addFlag(std::unique_ptr<IFlag> flag)
    {
        flags_.emplace_back(std::move(flag));
    }

    void addArg(std::unique_ptr<IArg> arg)
    {
        args_.emplace_back(std::move(arg));
    }

    void setArgList(std::unique_ptr<IArgList> argList)
    {
        if (argList_)
            configError_ = "Config can have only one arguments list";
        argList_ = std::move(argList);
    }

    void addCommand(std::unique_ptr<ICommand> command)
    {
        commands_.emplace_back(std::move(command));
    }

    std::vector<not_null<ICommand*>> commandList() const
    {
        return getPtrList(commands_);
    }

    void addHelpFlagToCommands(const std::string& commandName)
    {
        for (auto& cmd : commands_)
            cmd->enableHelpFlag(commandName);
    }


private:
    std::vector<std::unique_ptr<IParam>> params_;
    std::vector<std::unique_ptr<IParamList>> paramLists_;
    std::vector<std::unique_ptr<IFlag>> flags_;
    std::vector<std::unique_ptr<IArg>> args_;
    std::unique_ptr<IArgList> argList_;
    std::vector<std::unique_ptr<ICommand>> commands_;
    std::string versionInfo_;
    std::string customUsageInfo_;
    std::string customUsageInfoDetailed_;
    std::string configError_;

private:
    template<typename TConfig>
    friend class ConfigAccess;

    constexpr static FormatType format = formatType;
};

}
