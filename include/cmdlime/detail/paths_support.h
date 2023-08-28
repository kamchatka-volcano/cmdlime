#ifndef CMDLIME_PATHS_SUPPORT_H
#define CMDLIME_PATHS_SUPPORT_H

#include "external/sfun/path.h"
#include <cmdlime/stringconverter.h>
#include <filesystem>
#include <string>

namespace cmdlime {

template<>
struct StringConverter<std::filesystem::path> {
    static std::optional<std::string> toString(const std::filesystem::path& path)
    {
#ifdef CMDLIME_NO_WINDOWS_UNICODE_SUPPORT
        return path.string();
#else
        return sfun::path_string(path);
#endif
    }

    static std::optional<std::filesystem::path> fromString(const std::string& data)
    {
#ifdef CMDLIME_NO_WINDOWS_UNICODE_SUPPORT
#ifdef CMDLIME_NO_CANONICAL_PATHS
        return std::filesystem::path{data};
#else
        return std::filesystem::weakly_canonical(std::filesystem::path{data});
#endif
#else
#ifdef CMDLIME_NO_CANONICAL_PATHS
        return sfun::makePath(data);
#else
        return std::filesystem::weakly_canonical(sfun::make_path(data));
#endif
#endif
    }
};

} //namespace cmdlime

#endif //CMDLIME_PATHS_SUPPORT_H
