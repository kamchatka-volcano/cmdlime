#pragma once
#include <string>
#include <sstream>

namespace cmdlime::detail{

inline bool isNumber(const std::string& str)
{
    auto check = [&str](auto num){
        std::stringstream stream{str};
        stream >> num;
        return !stream.bad() && !stream.fail() && stream.eof();
    };
    return check(int64_t{}) || check(double{});
}

}
