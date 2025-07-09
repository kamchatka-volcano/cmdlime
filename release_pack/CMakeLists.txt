cmake_minimum_required(VERSION 3.18)
project(cmdlime VERSION 2.8.0 DESCRIPTION "C++17 command line parsing library")
include(seal_lake.cmake)

option(CMDLIME_USE_NAMEOF "Enable automatic registration of struct field names using the nameof library" ON)
option(CMDLIME_NO_WINDOWS_UNICODE "Disable storing std::wstring and std::filesystem::path with UTF16 encoding on Windows" OFF)

if (CMDLIME_USE_NAMEOF AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CPP_STANDARD_FEATURE cxx_std_20)
else ()
    set(CPP_STANDARD_FEATURE cxx_std_17)
endif ()

SealLake_V040_HeaderOnlyLibrary(
        COMPILE_FEATURES ${CPP_STANDARD_FEATURE}
)

if (CMDLIME_USE_NAMEOF)
    target_compile_definitions(cmdlime INTERFACE CMDLIME_USE_NAMEOF)
endif ()

if (CMDLIME_NO_WINDOWS_UNICODE)
    target_compile_definitions(cmdlime INTERFACE CMDLIME_NO_WINDOWS_UNICODE_SUPPORT)
endif ()

if (CMDLIME_NO_CANONICAL_PATHS)
    target_compile_definitions(cmdlime INTERFACE CMDLIME_NO_CANONICAL_PATHS)
endif ()