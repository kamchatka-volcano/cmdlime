#pragma once
#include <string>
#include <algorithm>

namespace str{

inline bool startsWith(const std::string &str, const std::string &val)
{
    auto res = str.find(val);
    return res == 0;
}

inline std::string after(const std::string &str, const std::string& val)
{
    auto res = str.find(val);
    if (res == std::string::npos)
        return {};
    return std::string(str.begin() + static_cast<int>(res + val.size()), str.end());
}

inline std::string before(const std::string &str, const std::string& val)
{
    auto res = str.find(val);
    if (res == std::string::npos)
        return str;
    return std::string(str.begin(), str.begin() + static_cast<int>(res));
}

inline std::string trimmedFront(const std::string& str)
{
    auto res = std::string(str);
    res.erase(res.begin(), std::find_if(res.begin(), res.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    return res;
}

};
