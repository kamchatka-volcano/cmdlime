cmake_minimum_required(VERSION 3.18)
project(cmdlime VERSION 1.1.0 DESCRIPTION "C++17 command line parsing library")
include(GNUInstallDirs)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMDLIME_SUBPROJECT OFF)
else()
    set(CMDLIME_SUBPROJECT ON)
endif()

option(CMDLIME_USE_NAMEOF "Enable automatic registration of struct field names using the nameof library" OFF)
if (CMDLIME_USE_NAMEOF)
    include(external/nameof)
endif()
include (external/sfun)
include (external/gsl)

add_library(cmdlime INTERFACE)
add_library(cmdlime::cmdlime ALIAS cmdlime)
target_compile_features(cmdlime INTERFACE cxx_std_17)

target_include_directories(
    cmdlime
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(
    cmdlime
    INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cmdlime/detail/external>
)
target_include_directories(
    cmdlime
    INTERFACE
    $<BUILD_INTERFACE:${sfun_SOURCE_DIR}/include/>
)
target_include_directories(
    cmdlime
    INTERFACE
    $<BUILD_INTERFACE:${gsl_SOURCE_DIR}/include/>
)
if (CMDLIME_USE_NAMEOF)
    target_include_directories(
        cmdlime
        INTERFACE
        $<BUILD_INTERFACE:${nameof_SOURCE_DIR}/include/>
    )
endif()

option(ENABLE_TESTS "Enable tests" OFF)
if (ENABLE_TESTS AND NOT CMDLIME_SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()

option(INSTALL_CMDLIME "Install cmdlime library unconditionally" OFF)
if (NOT CMDLIME_SUBPROJECT OR INSTALL_CMDLIME)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/cmdlime    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY ${sfun_SOURCE_DIR}/include/sfun          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmdlime/detail/external)
    install(DIRECTORY ${gsl_SOURCE_DIR}/include/gsl            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmdlime/detail/external)
    install(FILES     ${nameof_SOURCE_DIR}/include/nameof.hpp  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmdlime/detail/external)
    set(CMDLIME_PACK_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/cmdlime)

    install(TARGETS cmdlime
            EXPORT cmdlime-targets
            )
    install(EXPORT cmdlime-targets
            FILE cmdlimeTargets.cmake
            NAMESPACE cmdlime::
            DESTINATION ${CMDLIME_PACK_PATH}
            )
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/cmdlimeConfigVersion.cmake
            COMPATIBILITY SameMajorVersion
            ARCH_INDEPENDENT
    )
    configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/cmdlimeConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/cmdlimeConfig.cmake
            INSTALL_DESTINATION ${CMDLIME_PACK_PATH}
            )
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/cmdlimeConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/cmdlimeConfigVersion.cmake
            DESTINATION ${CMDLIME_PACK_PATH}
            )
endif()