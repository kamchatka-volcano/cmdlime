#ifndef CMDLIME_NAMEUTILS_H
#define CMDLIME_NAMEUTILS_H

#include "external/sfun/string_utils.h"
#include <algorithm>
#include <string>
#include <type_traits>

namespace cmdlime::detail {

namespace util {

inline std::string formatName(const std::string& name)
{
    auto result = name;
    //remove front non-alphabet characters
    result.erase(
            result.begin(),
            std::find_if(
                    result.begin(),
                    result.end(),
                    [](char ch)
                    {
                        return sfun::isalpha(ch);
                    }));
    //remove back non-alphabet and non-digit characters
    result.erase(
            std::find_if(
                    result.rbegin(),
                    result.rend(),
                    [](char ch)
                    {
                        return sfun::isalnum(ch);
                    })
                    .base(),
            result.end());
    return result;
}
} //namespace util

inline std::string toCamelCase(const std::string& name)
{
    auto result = std::string{};
    auto prevCharNonAlpha = false;
    auto formattedName = util::formatName(name);
    if (!formattedName.empty())
        formattedName[0] = sfun::tolower(formattedName[0]);
    for (auto ch : formattedName) {
        if (!std::isalpha(ch)) {
            if (std::isdigit(ch))
                result.push_back(ch);
            if (!result.empty())
                prevCharNonAlpha = true;
            continue;
        }
        if (prevCharNonAlpha)
            ch = sfun::toupper(ch);
        result.push_back(ch);
        prevCharNonAlpha = false;
    }
    return result;
}

inline std::string toKebabCase(const std::string& name)
{
    auto result = std::string{};
    auto formattedName = util::formatName(sfun::replace(name, "_", "-"));
    if (!formattedName.empty())
        formattedName[0] = sfun::tolower(formattedName[0]);
    for (auto ch : formattedName) {
        if (std::isupper(ch) && !result.empty()) {
            result.push_back('-');
            result.push_back(sfun::tolower(ch));
        }
        else
            result.push_back(ch);
    }
    return result;
}

inline std::string toLowerCase(const std::string& name)
{
    auto result = std::string{};
    for (auto ch : util::formatName(name)) {
        if (sfun::isalnum(ch))
            result.push_back(sfun::tolower(ch));
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
    auto result = sfun::between(type, "<", ">");
    if (!result.has_value())
        return type;
    return std::string{result.value()};
}

} //namespace cmdlime::detail

#endif //CMDLIME_NAMEUTILS_H