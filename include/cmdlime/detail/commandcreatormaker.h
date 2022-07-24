#pragma once
#include "commandcreator.h"
#include <cmdlime/config.h>

namespace cmdlime::detail{

template <typename T>
auto makeCommandCreator(ConfigReaderPtr cfgReader,
                        const std::string& varName,
                        const std::function<InitializedOptional<T>&()>& commandGetter,
                        typename Command<T>::Type type = Command<T>::Type::Normal)
{
    return CommandCreator<T>{cfgReader, varName, commandGetter(), type};
}


}