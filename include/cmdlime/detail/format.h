#pragma once

namespace cmdlime::detail{

enum class FormatType{
    Simple,
    POSIX,
    X11,
    GNU
};

template<FormatType>
struct Format;

template<>
struct Format<FormatType::Simple>;

template<>
struct Format<FormatType::POSIX>;

template<>
struct Format<FormatType::X11>;

template<>
struct Format<FormatType::GNU>;

}
