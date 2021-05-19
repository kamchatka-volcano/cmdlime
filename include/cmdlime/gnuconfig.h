#pragma once
#include "customnames.h"
#include "configreader.h"
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/gnuformat.h"

namespace cmdlime{
using GNUConfig = cmdlime::detail::Config<cmdlime::detail::FormatType::GNU>;
}
