#pragma once
#include "customnames.h"
#include "configreader.h"
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/posixformat.h"

namespace cmdlime{
using POSIXConfig = cmdlime::detail::Config<cmdlime::detail::FormatType::POSIX>;
}
