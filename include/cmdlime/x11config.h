#pragma once
#include "customnames.h"
#include "configreader.h"
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/x11format.h"

namespace cmdlime{
using X11Config = cmdlime::detail::Config<cmdlime::detail::FormatType::X11>;
}
