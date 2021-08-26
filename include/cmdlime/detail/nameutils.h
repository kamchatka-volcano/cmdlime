#pragma once
#include <sfun/string_utils.h>
#include <string>
#include <algorithm>

namespace cmdlime::detail{
namespace str = sfun::string_utils;

namespace util{
inline std::string formatName(const std::string& name)
{
    auto result = name;
    //remove front non-alphabet characters
    result.erase(result.begin(), std::find_if(result.begin(), result.end(),
        [](int ch){
            return std::isalpha(ch);
        })
    );
    //remove back non-alphabet and non-digit characters
    result.erase(std::find_if(result.rbegin(), result.rend(),
        [](int ch){ return std::isalnum(ch);}).base(), result.end());
    return result;
}
}

inline std::string toCamelCase(const std::string& name)
{
    auto result = std::string{};
    auto prevCharNonAlpha = false;
    auto formattedName = util::formatName(name);
    if (!formattedName.empty())
        formattedName[0] = static_cast<char>(std::tolower(formattedName[0]));
    for (auto ch : formattedName){
        if (!std::isalpha(ch)){
            if (std::isdigit(ch))
                result.push_back(ch);
            if (!result.empty())
                prevCharNonAlpha = true;
            continue;
        }
        if (prevCharNonAlpha)
            ch = static_cast<char>(std::toupper(ch));
        result.push_back(ch);
        prevCharNonAlpha = false;
    }
    return result;
}

inline std::string toKebabCase(const std::string& name)
{
    auto result = std::string{};
    auto formattedName = util::formatName(str::replace(name, "_", "-"));
    if (!formattedName.empty())
        formattedName[0] = static_cast<char>(std::tolower(formattedName[0]));
    for (auto ch : formattedName){
        if (std::isupper(ch) && !result.empty()){
            result.push_back('-');
            result.push_back(static_cast<char>(std::tolower(ch)));
        }
        else
            result.push_back(ch);
    }
    return result;
}

inline std::string toLowerCase(const std::string& name)
{
    auto result = std::string{};
    for (auto ch : util::formatName(name)){
        if (std::isalnum(ch))
            result.push_back(static_cast<char>(std::tolower(ch)));
    }
    return result;
}

inline std::string typeNameWithoutNamespace(const std::string& type)
{
    auto pos = type.rfind(':');
    if (pos == std::string::npos || pos == type.size() - 1)
        return type;
    return std::string{type.begin() + static_cast<int>(pos + 1), type.end()};
}

inline std::string templateType(const std::string& type)
{
    if (type.find('<') != std::string::npos)
        return str::before(str::after(type, "<"), ">");
    return type;
}

}
