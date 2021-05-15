#pragma once

namespace cmdlime::detail{

enum class FormatType{
    Default,
    POSIX,
    X11,
    GNU
};

template<FormatType>
struct Format;

template<>
struct Format<FormatType::Default>;

template<>
struct Format<FormatType::POSIX>;

template<>
struct Format<FormatType::X11>;

template<>
struct Format<FormatType::GNU>;

}
