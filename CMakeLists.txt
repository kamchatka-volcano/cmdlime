cmake_minimum_required(VERSION 3.18)
project(cmdlime VERSION 2.1.1 DESCRIPTION "C++17 command line parsing library")
include(GNUInstallDirs)
include(external/seal_lake)

option(CMDLIME_USE_NAMEOF "Enable automatic registration of struct field names using the nameof library" OFF)
if (CMDLIME_USE_NAMEOF)
    set(NAMEOF_OPT_INSTALL ON)
    SealLake_Bundle(
            NAME nameof
            GIT_REPOSITORY https://github.com/Neargye/nameof.git
            GIT_TAG        bc183cd
    )
endif()

SealLake_Bundle(
        NAME cmdlime_sfun
        GIT_REPOSITORY https://github.com/kamchatka-volcano/sfun.git
        GIT_TAG        v3.0.0
        DESTINATION include/cmdlime/detail/external
        DIRECTORIES include/sfun
        TEXT_REPLACEMENTS
            "namespace sfun" "namespace cmdlime::sfun"
            "SFUN_" "CMDLIME_SFUN_"
)

if (CMDLIME_USE_NAMEOF AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CPP_STANDARD_FEATURE cxx_std_20)
else()
    set(CPP_STANDARD_FEATURE cxx_std_17)
endif()

SealLake_HeaderOnlyLibrary(
        COMPILE_FEATURES ${CPP_STANDARD_FEATURE}
)

if (CMDLIME_USE_NAMEOF)
    SealLake_Libraries(
            nameof::nameof
    )
    SealLake_Dependencies(
            nameof 0.10.2
    )
endif()

SealLake_OptionalBuildSteps(tests examples)