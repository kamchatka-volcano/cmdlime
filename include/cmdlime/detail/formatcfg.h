#ifndef CMDLIME_FORMATCFG_H
#define CMDLIME_FORMATCFG_H

#include <cmdlime/format.h>

namespace cmdlime::detail {

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

} //namespace cmdlime::detail

#endif //CMDLIME_FORMATCFG_H