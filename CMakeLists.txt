cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED CMDLIME_SUBPROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(CMDLIME_SUBPROJECT OFF)
    else()
        set(CMDLIME_SUBPROJECT ON)
    endif()
endif()


project(cmdlime VERSION 1.0.0 DESCRIPTION "C++17 command line parsing library")
include(external/gsl)
include(external/sfun)

add_library(cmdlime INTERFACE)
target_compile_features(cmdlime INTERFACE cxx_std_17)
target_link_libraries(cmdlime INTERFACE GSL sfun)

target_include_directories(
    cmdlime
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(ENABLE_TESTS "Enable tests" OFF)
if (ENABLE_TESTS AND NOT CMDLIME_SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
if(NOT CMDLIME_SUBPROJECT)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/cmdlime DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY "${gsl_SOURCE_DIR}/include/gsl"
            COMPONENT gsl
            EXCLUDE_FROM_ALL
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY "${sfun_SOURCE_DIR}/include/sfun"
            COMPONENT sfun
            EXCLUDE_FROM_ALL
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
