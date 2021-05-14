#pragma once
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/x11format.h"
#include "detail/customnames.h"

namespace cmdlime{
using X11Config = cmdlime::detail::Config<cmdlime::detail::FormatType::X11>;
}
