#pragma once
#include "config.h"
#include "commandcreator.h"

namespace cmdlime::detail{

template <typename T, typename TConfig>
CommandCreator<T, TConfig> makeCommandCreator(TConfig& cfg,
                                              const std::string& varName,
                                              const std::function<std::optional<T>&()>& commandGetter,
                                              typename Command<T>::Type type = Command<T>::Type::Normal)
{
    static_assert (std::is_base_of_v<Config<ConfigAccess<TConfig>::format()>, T>,
                   "Command's type must be a subclass of Config<FormatType> and have the same format as its parent config.");
    return CommandCreator<T, TConfig>{cfg, varName, commandGetter(), type};
}


}