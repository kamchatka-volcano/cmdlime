#pragma once

namespace cmdlime::detail{

enum class FormatType{
    Default,
    POSIX,
};

template<FormatType>
struct Format;

template<>
struct Format<FormatType::Default>;

template<>
struct Format<FormatType::POSIX>;

}
