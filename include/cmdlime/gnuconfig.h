#pragma once
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/gnuformat.h"
#include "detail/customnames.h"

namespace cmdlime{
using GNUConfig = cmdlime::detail::Config<cmdlime::detail::FormatType::GNU>;
}
