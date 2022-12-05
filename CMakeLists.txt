cmake_minimum_required(VERSION 3.18)
project(cmdlime VERSION 2.1.0 DESCRIPTION "C++17 command line parsing library")
include(GNUInstallDirs)
include(external/seal_lake)

option(CMDLIME_USE_NAMEOF "Enable automatic registration of struct field names using the nameof library" OFF)
if (CMDLIME_USE_NAMEOF)
    set(NAMEOF_OPT_INSTALL ON)
    SealLake_Bundle(
            NAME nameof
            GIT_REPOSITORY https://github.com/Neargye/nameof.git
            GIT_TAG        f7b2f34
    )
endif()

SealLake_Bundle(
        NAME cmdlime_sfun
        GIT_REPOSITORY https://github.com/kamchatka-volcano/sfun.git
        GIT_TAG        v2.4.0
        DESTINATION include/cmdlime/detail/external
        DIRECTORIES include/sfun
        TEXT_REPLACEMENTS
            "namespace sfun" "namespace cmdlime::sfun"
            "SFUN_" "CMDLIME_SFUN_"
)

SealLake_HeaderOnlyLibrary(
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
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