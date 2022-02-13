#pragma once
#include "paramcreator.h"
#include "paramlistcreator.h"
#include "flagcreator.h"
#include "argcreator.h"
#include "arglistcreator.h"
#include "commandcreator.h"
#include "format.h"
#include "usageinfocreator.h"
#include "configaccess.h"
#include "utils.h"
#include "nameof_import.h"
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

protected:
    template<auto TMember>
    auto param(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(TMember){};
        return param<TMember>(ptr, memberName, memberTypeName);
    }

    template<auto TMember>
    auto paramList(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(TMember){};
        return paramList<TMember>(ptr, memberName, memberTypeName);
    }

    template<auto TMember>
    auto flag(const std::string& memberName)
    {
        auto ptr = decltype(TMember){};
        return flag<TMember>(ptr, memberName);
    }

    template<auto TMember>
    auto exitFlag(const std::string& memberName)
    {
        auto ptr = decltype(TMember){};
        return exitFlag<TMember>(ptr, memberName);
    }

    template<auto TMember>
    auto arg(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(TMember){};
        return arg<TMember>(ptr, memberName, memberTypeName);
    }

    template<auto TMember>
    auto argList(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(TMember){};
        return argList<TMember>(ptr, memberName, memberTypeName);
    }

    template<auto TMember>
    auto command(const std::string& memberName)
    {
        auto ptr = decltype(TMember){};
        return command<TMember>(ptr, memberName);
    }

    template<auto TMember>
    auto subCommand(const std::string& memberName)
    {
        auto ptr = decltype(TMember){};
        return subCommand<TMember>(ptr, memberName);
    }

#ifdef CMDLIME_NAMEOF_AVAILABLE
    template<auto TMember>
    auto param()
    {
        auto ptr = decltype(TMember){};
        return param<TMember>(ptr);
    }

    template<auto TMember>
    auto paramList()
    {
        auto ptr = decltype(TMember){};
        return paramList<TMember>(ptr);
    }

    template<auto TMember>
    auto flag()
    {
        auto ptr = decltype(TMember){};
        return flag<TMember>(ptr);
    }

    template<auto TMember>
    auto exitFlag()
    {
        auto ptr = decltype(TMember){};
        return exitFlag<TMember>(ptr);
    }

    template<auto TMember>
    auto arg()
    {
        auto ptr = decltype(TMember){};
        return arg<TMember>(ptr);
    }

    template<auto TMember>
    auto argList()
    {
        auto ptr = decltype(TMember){};
        return argList<TMember>(ptr);
    }

    template<auto TMember>
    auto command()
    {
        auto ptr = decltype(TMember){};
        return command<TMember>(ptr);
    }

    template<auto TMember>
    auto subCommand()
    {
        auto ptr = decltype(TMember){};
        return subCommand<TMember>(ptr);
    }
#endif

private:

    template <auto member, typename T, typename TCfg>
    detail::ParamCreator<T, TCfg> param(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::ParamListCreator<T, TCfg> paramList(std::vector<T> TCfg::*,
                                                const std::string& memberName,
                                                const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamListCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    detail::FlagCreator<TCfg> flag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator<TCfg>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    detail::FlagCreator<TCfg> exitFlag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator<TCfg>{*cfg, memberName, cfg->*member, detail::Flag::Type::Exit};
    }


    template <auto member, typename T, typename TCfg>
    detail::ArgCreator<T, TCfg> arg(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::ArgListCreator<T, TCfg> argList(std::vector<T> TCfg::*,
                                            const std::string& memberName,
                                            const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgListCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::CommandCreator<T, TCfg> command(std::optional<T> TCfg::*, const std::string& memberName)
    {
        static_assert(std::is_base_of_v<Config<ConfigAccess<TCfg>::format()>, T>,
                      "Command's type must be a subclass of Config<FormatType> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T, TCfg>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::CommandCreator<T, TCfg> subCommand(std::optional<T> TCfg::*, const std::string& memberName)
    {
        static_assert(std::is_base_of_v<Config<ConfigAccess<TCfg>::format()>, T>,
                      "Command's type must be a subclass of Config<FormatType> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T, TCfg>{*cfg, memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }

#ifdef CMDLIME_NAMEOF_AVAILABLE
    template <auto member, typename T, typename TCfg>
    detail::ParamCreator<T, TCfg> param(T TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = getMemberPtrNameAndType<member>(cfg);
        return detail::ParamCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::ParamListCreator<T, TCfg> paramList(std::vector<T> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = nameOfType<T>();
        return detail::ParamListCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    detail::FlagCreator<TCfg> flag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    detail::FlagCreator<TCfg> exitFlag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator{*cfg, memberName, cfg->*member, detail::Flag::Type::Exit};
    }

    template <auto member, typename T, typename TCfg>
    detail::ArgCreator<T, TCfg> arg(T TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = getMemberPtrNameAndType<member>(cfg);
        return detail::ArgCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::ArgListCreator<T, TCfg> argList(std::vector<T> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = nameOfType<T>();
        return detail::ArgListCreator<T, TCfg>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::CommandCreator<T, TCfg> command(std::optional<T> TCfg::*)
    {
        static_assert(std::is_base_of_v<Config<ConfigAccess<TCfg>::format()>, T>,
                      "Command's type must be a subclass of Config<FormatType> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<T, TCfg>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    detail::CommandCreator<T, TCfg> subCommand(std::optional<T> TCfg::*)
    {
        static_assert(std::is_base_of_v<Config<ConfigAccess<TCfg>::format()>, T>,
                      "Command's type must be a subclass of Config<FormatType> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<T, TCfg>{*cfg, memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }
#endif

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
