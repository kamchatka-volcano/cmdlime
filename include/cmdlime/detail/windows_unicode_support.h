#ifndef CMDLIME_WINDOWS_UNICODE_SUPPORT_H
#define CMDLIME_WINDOWS_UNICODE_SUPPORT_H

#ifdef _WIN32
#ifndef CMDLIME_NO_WINDOWS_UNICODE_SUPPORT

#include "external/sfun/path.h"
#include "external/sfun/wstringconv.h"
#include <cmdlime/stringconverter.h>
#include <filesystem>
#include <string>

namespace cmdlime {

template<>
struct StringConverter<std::filesystem::path> {
    static std::optional<std::string> toString(const std::filesystem::path& path)
    {
        return sfun::pathString(path);
    }

    static std::optional<std::filesystem::path> fromString(const std::string& data)
    {
        return sfun::makePath(data);
    }
};

template<>
struct StringConverter<std::wstring> {
    static std::optional<std::string> toString(const std::wstring& str)
    {
        return sfun::fromWString(str);
    }

    static std::optional<std::wstring> fromString(const std::string& data)
    {
        return sfun::toWString(data);
    }
};

} //namespace cmdlime

#endif
#endif
#endif //CMDLIME_WINDOWS_UNICODE_SUPPORT_H