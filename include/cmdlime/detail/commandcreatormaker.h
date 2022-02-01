#pragma once
#include "commandcreator.h"
#include <cmdlime/baseconfig.h>

namespace cmdlime::detail{

template <typename T, typename TConfig>
auto makeCommandCreator(TConfig& cfg,
                        const std::string& varName,
                        const std::function<std::optional<T>&()>& commandGetter,
                        typename Command<T>::Type type = Command<T>::Type::Normal)
{
    static_assert (std::is_base_of_v<BaseConfig<TConfig::format()>, T>,
                   "Command's type must be a subclass of BaseConfig<Format> and have the same format as its parent config.");
    return CommandCreator<T, TConfig::format()>{cfg, varName, commandGetter(), type};
}


}