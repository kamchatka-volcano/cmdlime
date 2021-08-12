#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>

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
    return std::string{str.begin() + static_cast<int>(res + val.size()), str.end()};
}

inline std::string before(const std::string &str, const std::string& val)
{
    auto res = str.find(val);
    if (res == std::string::npos)
        return str;
    return std::string{str.begin(), str.begin() + static_cast<int>(res)};
}

inline std::string trimmedFront(const std::string& str)
{
    auto res = std::string(str);
    res.erase(res.begin(), std::find_if(res.begin(), res.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    return res;
}

inline std::string trimmed(const std::string& str)
{
    auto res = std::string(str);
    res.erase(res.begin(), std::find_if(res.begin(), res.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    res.erase(std::find_if(res.rbegin(), res.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), res.end());
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

inline std::vector<std::string> split(const std::string& str, char delimiter, bool trim = true)
{
    auto result = std::vector<std::string>{};
    auto stream = std::stringstream{str};
    auto part = std::string{};
    while(std::getline(stream, part, delimiter))
        if (trim)
            result.push_back(trimmed(part));
        else
            result.push_back(part);

    return result;
}

}
