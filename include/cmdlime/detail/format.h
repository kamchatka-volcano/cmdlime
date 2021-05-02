#pragma once

namespace cmdlime::detail{

enum class FormatType{
    Default,
};

template<FormatType>
struct Format;

template<>
struct Format<FormatType::Default>;

}
