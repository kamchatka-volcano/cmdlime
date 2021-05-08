#pragma once
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/defaultformat.h"
#include "detail/customnames.h"

namespace cmdlime{
using Config = cmdlime::detail::Config<cmdlime::detail::FormatType::Default>;
}
