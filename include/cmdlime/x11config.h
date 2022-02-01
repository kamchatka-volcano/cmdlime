#pragma once
#include "customnames.h"
#include "configreader.h"
#include "stringconverter.h"
#include "baseconfig.h"
#include "detail/configmacro.h"
#include "detail/x11format.h"

namespace cmdlime{
using X11Config = cmdlime::BaseConfig<cmdlime::Format::X11>;
}
