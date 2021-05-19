#pragma once
#include "customnames.h"
#include "configreader.h"
#include "detail/config.h"
#include "detail/configmacro.h"
#include "detail/simpleformat.h"

namespace cmdlime{
using SimpleConfig = cmdlime::detail::Config<cmdlime::detail::FormatType::Simple>;
}
