#pragma once
#include <cmdlime/format.h>

namespace cmdlime::detail{

template<Format>
struct FormatCfg;

template<>
struct FormatCfg<Format::Simple>;

template<>
struct FormatCfg<Format::POSIX>;

template<>
struct FormatCfg<Format::X11>;

template<>
struct FormatCfg<Format::GNU>;

}