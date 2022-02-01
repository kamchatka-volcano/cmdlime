#pragma once
#include "customnames.h"
#include "configreader.h"
#include "stringconverter.h"
#include "baseconfig.h"
#include "detail/configmacro.h"
#include "detail/gnuformat.h"

namespace cmdlime{
using GNUConfig = cmdlime::BaseConfig<cmdlime::Format::GNU>;
}
