cmake_minimum_required(VERSION 3.18)
project(cmdlime VERSION 2.0.2 DESCRIPTION "C++17 command line parsing library")
include(GNUInstallDirs)
include(external/seal_lake)

option(CMDLIME_USE_NAMEOF "Enable automatic registration of struct field names using the nameof library" OFF)
if (CMDLIME_USE_NAMEOF)
    SealLake_FindOrDownload(
            nameof 0.10.1
            GIT_REPOSITORY https://github.com/Neargye/nameof.git
            GIT_TAG        v0.10.1
    )
endif()
SealLake_FindOrDownload(
        sfun 2.4.0
        GIT_REPOSITORY https://github.com/kamchatka-volcano/sfun.git
        GIT_TAG        cmake_maintenance
)
SealLake_FindOrDownload(
        GSL 4.0.0
        GIT_REPOSITORY https://github.com/microsoft/GSL
        GIT_TAG        v4.0.0
)

SealLake_HeaderOnlyLibrary(
        COMPILE_FEATURES cxx_std_17
        INCLUDES cmdlime/detail/external
        BUILD_STAGE_INCLUDES
            ${sfun_SOURCE_DIR}/include/
            ${GSL_SOURCE_DIR}/include/
)
if (CMDLIME_USE_NAMEOF)
    SealLake_BuildStageIncludes(${nameof_SOURCE_DIR}/include/)
endif()

SealLake_OptionalBuildSteps(
        IF_ENABLED_AND_STANDALONE
            tests
            examples
)

SealLake_Install(
        PATH detail/external
        DIRECTORIES
            ${sfun_SOURCE_DIR}/include/sfun
            ${GSL_SOURCE_DIR}/include/gsl
)
if (CMDLIME_USE_NAMEOF)
    SealLake_Install(
            FILES
                ${nameof_SOURCE_DIR}/include/nameof.hpp
    )
endif()