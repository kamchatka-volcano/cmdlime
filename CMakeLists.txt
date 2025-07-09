cmake_minimum_required(VERSION 3.18)
project(cmdlime VERSION 2.8.0 DESCRIPTION "C++17 command line parsing library")
include(external/seal_lake)

option(CMDLIME_USE_NAMEOF "Enable automatic registration of struct field names using the nameof library" ON)
option(CMDLIME_NO_WINDOWS_UNICODE "Disable storing std::wstring and std::filesystem::path with UTF16 encoding on Windows" OFF)

if (CMDLIME_USE_NAMEOF)
    SealLake_V040_Bundle(
            NAME cmdlime_nameof
            GIT_REPOSITORY https://github.com/Neargye/nameof.git
            GIT_TAG v0.10.4
            SKIP_LOAD
            FILES include/nameof.hpp
            DESTINATION include/cmdlime/detail/external/nameof
            TEXT_REPLACEMENTS
            "namespace nameof" "namespace cmdlime::nameof"
            "nameof::" "cmdlime::nameof::"
            "NAMEOF_" "CMDLIME_NAMEOF_"
    )
endif()

SealLake_V040_Bundle(
        NAME cmdlime_eel
        URL https://github.com/kamchatka-volcano/eel/releases/download/v0.3.0/eel-v0.3.0.zip
        DESTINATION include/cmdlime/detail/external
        DIRECTORIES include/eel
        TEXT_REPLACEMENTS
            "namespace eel" "namespace cmdlime::eel"
            "EEL_" "CMDLIME_EEL_"
)

if (CMDLIME_USE_NAMEOF AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CPP_STANDARD_FEATURE cxx_std_20)
else()
    set(CPP_STANDARD_FEATURE cxx_std_17)
endif()

SealLake_V040_HeaderOnlyLibrary(
        COMPILE_FEATURES ${CPP_STANDARD_FEATURE}
)

if (CMDLIME_USE_NAMEOF)
    target_compile_definitions(cmdlime INTERFACE CMDLIME_USE_NAMEOF)
endif ()

if (CMDLIME_NO_WINDOWS_UNICODE)
    target_compile_definitions(cmdlime INTERFACE CMDLIME_NO_WINDOWS_UNICODE_SUPPORT)
endif()

if (CMDLIME_NO_CANONICAL_PATHS)
    target_compile_definitions(cmdlime INTERFACE CMDLIME_NO_CANONICAL_PATHS)
endif()

SealLake_V040_OptionalSubProjects(tests examples)