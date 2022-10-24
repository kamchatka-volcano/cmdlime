#ifndef CMDLIME_CONFIG_H
#define CMDLIME_CONFIG_H

#include "customnames.h"
#include "detail/configmacros.h"
#include "detail/paramcreator.h"
#include "detail/paramlistcreator.h"
#include "detail/flagcreator.h"
#include "detail/argcreator.h"
#include "detail/arglistcreator.h"
#include "detail/commandcreator.h"
#include "detail/icommandlinereader.h"
#include "detail/nameof_import.h"
#include "detail/ivalidator.h"
#include <optional>
#include <string>

namespace cmdlime{

class Config {
public:
    Config() = default;
    Config(detail::CommandLineReaderPtr reader)
        : reader_{reader}
    {}
    Config(const Config&) = default;
    Config& operator=(const Config&) = default;
    Config(Config&&)
    {
    }
    Config& operator=(Config&&)
    {
        return *this;
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

    detail::CommandLineReaderPtr reader() const
    {
        return reader_;
    }

private:
#ifdef CMDLIME_NAMEOF_AVAILABLE
    template <auto member, typename T, typename TCfg>
    auto param(T TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::ParamCreator<T>{reader(), memberName, memberTypeName, cfg->*member};

    }

    template <auto member, typename TParamList, typename TCfg>
    auto paramList(TParamList TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = detail::nameOfType<TParamList>();
        return detail::ParamListCreator<TParamList>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto flag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator{reader(), memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto exitFlag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator{reader(), memberName, cfg->*member, detail::Flag::Type::Exit};
    }

    template <auto member, typename T, typename TCfg>
    auto arg(T TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::ArgCreator<T>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TArgList, typename TCfg>
    auto argList(TArgList TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = detail::nameOfType<TArgList>();
        return detail::ArgListCreator<TArgList>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto command(detail::InitializedOptional<T> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<T>{reader(), memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto subCommand(detail::InitializedOptional<T> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<T>{reader(), memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }
#endif

    template <auto member, typename T, typename TCfg>
    auto param(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamCreator<T>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TParamList, typename TCfg>
    auto paramList(TParamList TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamListCreator<TParamList>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto flag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator{reader(), memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto exitFlag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator{reader(), memberName, cfg->*member, detail::Flag::Type::Exit};
    }


    template <auto member, typename T, typename TCfg>
    auto arg(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgCreator<T>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TArgList, typename TCfg>
    auto argList(TArgList TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgListCreator<TArgList>{reader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto command(detail::InitializedOptional<T> TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T>{reader(), memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto subCommand(detail::InitializedOptional<T> TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T>{reader(), memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }


private:
    detail::CommandLineReaderPtr reader_;
    friend class detail::ICommandLineReader;
};

template<typename T>
using optional = std::conditional_t<std::is_base_of_v<Config, T>, detail::InitializedOptional<T>, std::optional<T>>;

}

#endif //CMDLIME_CONFIG_H