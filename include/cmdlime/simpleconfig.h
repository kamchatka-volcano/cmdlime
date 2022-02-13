#pragma once
#include "customnames.h"
#include "configreader.h"
#include "stringconverter.h"
#include "baseconfig.h"
#include "detail/configmacro.h"
#include "detail/simpleformat.h"

namespace cmdlime{
using SimpleConfig = cmdlime::BaseConfig<cmdlime::Format::Simple>;
}
