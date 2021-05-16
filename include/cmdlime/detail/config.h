#pragma once
#include "format.h"
#include "usageinfocreator.h"
#include "configaccess.h"
#include "usageinfoformat.h"
#include "gsl/pointers"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <memory>
#include <algorithm>

namespace cmdlime::detail{
using namespace gsl;

class IParam;
class IParamList;
class IFlag;
class IArg;
class IArgList;

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
        auto params = getPtrList(params_);
        auto paramLists = getPtrList(paramLists_);
        auto flags = getPtrList(flags_);
        auto args = getPtrList(args_);
        using ParserType = typename Format<formatType>::parser;
        auto parser = ParserType{params, paramLists, flags, args, argList_.get()};
        parser.parse(cmdLine);
    }

    std::string usageInfo(const std::string& name, UsageInfoFormat outputSettings = {})
    {
        auto params = getPtrList(params_);
        auto paramLists = getPtrList(paramLists_);
        auto flags = getPtrList(flags_);
        auto args = getPtrList(args_);
        return UsageInfoCreator<formatType>{name, outputSettings, params, paramLists, flags, args, argList_.get()}.create();
    }

    std::string usageInfoDetailed(const std::string& name, UsageInfoFormat outputSettings = {})
    {
        auto params = getPtrList(params_);
        auto paramLists = getPtrList(paramLists_);
        auto flags = getPtrList(flags_);
        auto args = getPtrList(args_);
        return UsageInfoCreator<formatType>{name, outputSettings, params, paramLists, flags, args, argList_.get()}.createDetailed();
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
        argList_ = std::move(argList);
    }


private:
    std::vector<std::unique_ptr<IParam>> params_;
    std::vector<std::unique_ptr<IParamList>> paramLists_;
    std::vector<std::unique_ptr<IFlag>> flags_;
    std::vector<std::unique_ptr<IArg>> args_;
    std::unique_ptr<detail::IArgList> argList_;

private:
    template<typename TConfig>
    friend class ConfigAccess;
    template<typename T, typename TConfig>
    friend class ParamCreator;
    template<typename T, typename TConfig>
    friend class ParamListCreator;
    template<typename TConfig>
    friend class FlagCreator;
    template<typename T, typename TConfig>
    friend class ArgCreator;
    template<typename T, typename TConfig>
    friend class ArgListCreator;

    constexpr static FormatType format = formatType;
};

}
