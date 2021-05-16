#pragma once
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/simpleformat.h"
#include "detail/customnames.h"

namespace cmdlime{
using SimpleConfig = cmdlime::detail::Config<cmdlime::detail::FormatType::Simple>;
}
