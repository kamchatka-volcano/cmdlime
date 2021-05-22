cmake_minimum_required(VERSION 3.12)
if(DEFINED PROJECT_NAME)
  set(CMDLIME_SUBPROJECT ON)
endif()

project(cmdlime VERSION 1.0.0 DESCRIPTION "C++17 command line parsing library")

include(GNUInstallDirs)

add_library(cmdlime INTERFACE)
target_compile_features(cmdlime INTERFACE cxx_std_17)
target_include_directories(
    cmdlime
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(ENABLE_TESTS "Enable tests" OFF)
if (${ENABLE_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

if(NOT CMDLIME_SUBPROJECT)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/cmdlime DESTINATION include)
endif()
