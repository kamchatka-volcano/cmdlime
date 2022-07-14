#pragma once
#include "usageinfoformat.h"
#include "errors.h"
#include "format.h"
#include "detail/paramcreator.h"
#include "detail/paramlistcreator.h"
#include "detail/flagcreator.h"
#include "detail/argcreator.h"
#include "detail/arglistcreator.h"
#include "detail/commandcreator.h"
#include "detail/iconfig.h"
#include "detail/options.h"
#include "detail/usageinfocreator.h"
#include "detail/utils.h"
#include "detail/nameof_import.h"
#include "detail/ivalidator.h"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <memory>
#include <algorithm>

namespace cmdlime{
using namespace gsl;

template<Format formatType>
class BaseConfig : public detail::IConfig{
public:
    void readCommandLine(int argc, char** argv)
    {
        auto cmdLine = std::vector<std::string>(argv + 1, argv + argc);
        readCommandLine(cmdLine);
    }

    void readCommandLine(const std::vector<std::string>& cmdLine)
    {
        if (read(cmdLine) != detail::ConfigReadResult::StoppedOnExitFlag)
            validate({});
    }

    const std::string& versionInfo() const override
    {
        return versionInfo_;
    }

    std::string usageInfo() const override
    {
        if (!customUsageInfo_.empty())
            return customUsageInfo_;

        return detail::UsageInfoCreator<formatType>{commandName_, usageInfoFormat_, options_}.create();
    }

    std::string usageInfoDetailed() const override
    {
        if (!customUsageInfoDetailed_.empty())
            return customUsageInfoDetailed_;

        return detail::UsageInfoCreator<formatType>{commandName_, usageInfoFormat_, options_}.createDetailed();
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

    void setProgramName(const std::string& name)
    {
        setCommandName(name);
    }

    void setUsageInfoFormat(const UsageInfoFormat& format) override
    {
        usageInfoFormat_ = format;
        for (auto& command : options_.commands())
            command->setUsageInfoFormat(format);
    }

    static constexpr Format format()
    {
        return formatType;
    }

protected:
    template<auto member>
    auto param(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(member){};
        return param<member>(ptr, memberName, memberTypeName);
    }

    template<auto member>
    auto paramList(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(member){};
        return paramList<member>(ptr, memberName, memberTypeName);
    }

    template<auto member>
    auto flag(const std::string& memberName)
    {
        auto ptr = decltype(member){};
        return flag<member>(ptr, memberName);
    }

    template<auto member>
    auto exitFlag(const std::string& memberName)
    {
        auto ptr = decltype(member){};
        return exitFlag<member>(ptr, memberName);
    }

    template<auto member>
    auto arg(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(member){};
        return arg<member>(ptr, memberName, memberTypeName);
    }

    template<auto member>
    auto argList(const std::string& memberName, const std::string& memberTypeName)
    {
        auto ptr = decltype(member){};
        return argList<member>(ptr, memberName, memberTypeName);
    }

    template<auto member>
    auto command(const std::string& memberName)
    {
        auto ptr = decltype(member){};
        return command<member>(ptr, memberName);
    }

    template<auto member>
    auto subCommand(const std::string& memberName)
    {
        auto ptr = decltype(member){};
        return subCommand<member>(ptr, memberName);
    }

#ifdef CMDLIME_NAMEOF_AVAILABLE
    template<auto member>
    auto param()
    {
        auto ptr = decltype(member){};
        return param<member>(ptr);
    }

    template<auto member>
    auto paramList()
    {
        auto ptr = decltype(member){};
        return paramList<member>(ptr);
    }

    template<auto member>
    auto flag()
    {
        auto ptr = decltype(member){};
        return flag<member>(ptr);
    }

    template<auto member>
    auto exitFlag()
    {
        auto ptr = decltype(member){};
        return exitFlag<member>(ptr);
    }

    template<auto member>
    auto arg()
    {
        auto ptr = decltype(member){};
        return arg<member>(ptr);
    }

    template<auto member>
    auto argList()
    {
        auto ptr = decltype(member){};
        return argList<member>(ptr);
    }

    template<auto member>
    auto command()
    {
        auto ptr = decltype(member){};
        return command<member>(ptr);
    }

    template<auto member>
    auto subCommand()
    {
        auto ptr = decltype(member){};
        return subCommand<member>(ptr);
    }
#endif

private:

    template <auto member, typename T, typename TCfg>
    auto param(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto paramList(std::vector<T> TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamListCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto flag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator<TCfg::format()>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto exitFlag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator<TCfg::format()>{*cfg, memberName, cfg->*member, detail::Flag::Type::Exit};
    }


    template <auto member, typename T, typename TCfg>
    auto arg(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto argList(std::vector<T> TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgListCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto command(std::optional<T> TCfg::*, const std::string& memberName)
    {
        static_assert(std::is_base_of_v<BaseConfig<TCfg::format()>, T>,
                      "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T, TCfg::format()>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto subCommand(std::optional<T> TCfg::*, const std::string& memberName)
    {
        static_assert(std::is_base_of_v<BaseConfig<TCfg::format()>, T>,
                      "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T, TCfg::format()>{*cfg, memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }

#ifdef CMDLIME_NAMEOF_AVAILABLE
    template <auto member, typename T, typename TCfg>
    auto param(T TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::ParamCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto paramList(std::vector<T> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = detail::nameOfType<T>();
        return detail::ParamListCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto flag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator<TCfg::format()>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto exitFlag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator<TCfg::format()>{*cfg, memberName, cfg->*member, detail::Flag::Type::Exit};
    }

    template <auto member, typename T, typename TCfg>
    auto arg(T TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::ArgCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto argList(std::vector<T> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = detail::nameOfType<T>();
        return detail::ArgListCreator<T, TCfg::format()>{*cfg, memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto command(std::optional<T> TCfg::*)
    {
        static_assert(std::is_base_of_v<BaseConfig<TCfg::format()>, T>,
                      "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<T, TCfg::format()>{*cfg, memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto subCommand(std::optional<T> TCfg::*)
    {
        static_assert(std::is_base_of_v<BaseConfig<TCfg::format()>, T>,
                      "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<T, TCfg::format()>{*cfg, memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }
#endif

    void addParam(std::unique_ptr<detail::IParam> param) override
    {
        options_.addParam(std::move(param));
    }

    void addParamList(std::unique_ptr<detail::IParamList> paramList) override
    {
        options_.addParamList(std::move(paramList));
    }

    void addFlag(std::unique_ptr<detail::IFlag> flag) override
    {
        options_.addFlag(std::move(flag));
    }

    void addArg(std::unique_ptr<detail::IArg> arg) override
    {
        options_.addArg(std::move(arg));
    }

    void setArgList(std::unique_ptr<detail::IArgList> argList) override
    {
        if (argListSet_) {
            configError_ = "BaseConfig can have only one arguments list";
            return;
        }
        options_.setArgList(std::move(argList));
        argListSet_ = true;
    }

    void addCommand(std::unique_ptr<detail::ICommand> command) override
    {
        options_.addCommand(std::move(command));
    }

    const detail::Options& options() const override
    {
        return options_;
    }

    void setCommandName(const std::string& name) override
    {
        commandName_ = name;
        for (auto& command : options_.commands())
            command->setCommandName(name);
    }

    void addValidator(std::unique_ptr<detail::IValidator> validator) override
    {
        validators_.emplace_back(std::move(validator));
    }

    void validate(const std::string& commandName) const override
    {
        auto commandIsSet = false;
        for (auto& command : options_.commands()) {
            command->validate();
            if (command->config() && !command->isSubCommand())
                commandIsSet = true;
        }
        for (auto& validator : validators_) {
            if (commandIsSet && validator->optionType() != detail::OptionType::Command)
                continue;
            validator->validate(commandName);
        }

    }

private:
    detail::ConfigReadResult read(const std::vector<std::string>& cmdLine) override
    {
        if (!configError_.empty())
            throw ConfigError{configError_};
        using ParserType = typename detail::FormatCfg<formatType>::parser;
        auto parser = ParserType{options_};
        return parser.parse(cmdLine);
    }

private:
    std::string versionInfo_;
    std::string customUsageInfo_;
    std::string customUsageInfoDetailed_;
    std::string configError_;
    detail::Options options_;
    std::string commandName_;
    UsageInfoFormat usageInfoFormat_;
    std::vector<std::unique_ptr<detail::IValidator>> validators_;
    bool argListSet_ = false;
};

}
