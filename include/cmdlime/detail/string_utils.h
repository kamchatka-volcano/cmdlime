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


inline std::string replace(const std::string &str, const std::string &subStr, const std::string &val)
{
    std::string res = std::string(str);
    auto pos = res.find(subStr);
    while (pos != std::string::npos){
        res.replace(pos, subStr.size(), val);
        pos = res.find(subStr, pos + val.size());
    }
    return res;
}

};
