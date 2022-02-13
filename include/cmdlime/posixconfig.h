#pragma once
#include "customnames.h"
#include "configreader.h"
#include "stringconverter.h"
#include "baseconfig.h"
#include "detail/configmacro.h"
#include "detail/posixformat.h"

namespace cmdlime{
using POSIXConfig = cmdlime::BaseConfig<cmdlime::Format::POSIX>;
}
