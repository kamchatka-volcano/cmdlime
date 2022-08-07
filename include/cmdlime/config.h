#pragma once
#include "format.h"
#include "errors.h"
#include "usageinfoformat.h"
#include "detail/configmacros.h"
#include "detail/paramcreator.h"
#include "detail/paramlistcreator.h"
#include "detail/flagcreator.h"
#include "detail/argcreator.h"
#include "detail/arglistcreator.h"
#include "detail/commandcreator.h"
#include "detail/iconfigreader.h"
#include "detail/options.h"
#include "detail/usageinfocreator.h"
#include "detail/utils.h"
#include "detail/nameof_import.h"
#include "detail/ivalidator.h"
#include <optional>
#include <string>

namespace cmdlime{

class Config {
public:
    Config() = default;
    Config(detail::ConfigReaderPtr reader)
        : cfgReader_{reader}
    {}
    Config(const Config&) = default;
    Config& operator=(const Config&) = default;
    Config(Config&& other)
    {
        if (cfgReader() && other.cfgReader())
            cfgReader()->swapContents(other.cfgReader());
    }
    Config& operator=(Config&& other)
    {
        if (cfgReader() && other.cfgReader())
            cfgReader()->swapContents(other.cfgReader());
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

    detail::ConfigReaderPtr cfgReader() const
    {
        return cfgReader_;
    }

private:
#ifdef CMDLIME_NAMEOF_AVAILABLE
    template <auto member, typename TCfg, typename TCfg>
    auto param(TCfg TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::ParamCreator<TCfg>{cfgReader(), memberName, memberTypeName, cfg->*member};

    }

    template <auto member, typename TCfg, typename TCfg>
    auto paramList(std::vector<TCfg> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = detail::nameOfType<TCfg>();
        return detail::ParamListCreator<TCfg>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto flag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator{cfgReader(), memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto exitFlag(bool TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::FlagCreator{cfgReader(), memberName, cfg->*member, detail::Flag::Type::Exit};
    }

    template <auto member, typename TCfg, typename TCfg>
    auto arg(TCfg TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, memberTypeName] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::ArgCreator<TCfg>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg, typename TCfg>
    auto argList(std::vector<TCfg> TCfg::*)
    {
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        const auto memberTypeName = detail::nameOfType<TCfg>();
        return detail::ArgListCreator<TCfg>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg, typename TCfg>
    auto command(detail::InitializedOptional<TCfg> TCfg::*)
    {
//        static_assert(std::is_base_of_v<BaseConfig<TCfg::format()>, TCfg>,
//                      "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<TCfg>{cfgReader(), memberName, cfg->*member};
    }

    template <auto member, typename TCfg, typename TCfg>
    auto subCommand(detail::InitializedOptional<TCfg> TCfg::*)
    {
//        static_assert(std::is_base_of_v<BaseConfig<TCfg::format()>, TCfg>,
//                      "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
        auto cfg = static_cast<TCfg*>(this);
        auto [memberName, _] = detail::getMemberPtrNameAndType<member>(cfg);
        return detail::CommandCreator<TCfg>{cfgReader(), memberName, cfg->*member, detail::Command<TCfg>::Type::SubCommand};
    }
#endif

    template <auto member, typename T, typename TCfg>
    auto param(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamCreator<T>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto paramList(std::vector<T> TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ParamListCreator<T>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto flag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator{cfgReader(), memberName, cfg->*member};
    }

    template <auto member, typename TCfg>
    auto exitFlag(bool TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::FlagCreator{cfgReader(), memberName, cfg->*member, detail::Flag::Type::Exit};
    }


    template <auto member, typename T, typename TCfg>
    auto arg(T TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgCreator<T>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto argList(std::vector<T> TCfg::*, const std::string& memberName, const std::string& memberTypeName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::ArgListCreator<T>{cfgReader(), memberName, memberTypeName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto command(detail::InitializedOptional<T> TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T>{cfgReader(), memberName, cfg->*member};
    }

    template <auto member, typename T, typename TCfg>
    auto subCommand(detail::InitializedOptional<T> TCfg::*, const std::string& memberName)
    {
        auto cfg = static_cast<TCfg*>(this);
        return detail::CommandCreator<T>{cfgReader(), memberName, cfg->*member, detail::Command<T>::Type::SubCommand};
    }


private:
    detail::ConfigReaderPtr cfgReader_;
    friend class detail::IConfigReader;
};

template<typename T>
using optional = std::conditional_t<std::is_base_of_v<Config, T>, detail::InitializedOptional<T>, std::optional<T>>;

}
