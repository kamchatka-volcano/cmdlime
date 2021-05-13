#pragma once
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/posixformat.h"
#include "detail/customnames.h"

namespace cmdlime{
using POSIXConfig = cmdlime::detail::Config<cmdlime::detail::FormatType::POSIX>;
}
