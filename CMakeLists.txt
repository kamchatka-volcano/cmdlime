cmake_minimum_required(VERSION 3.18)
if(DEFINED PROJECT_NAME)
  set(CMDLIME_SUBPROJECT ON)
endif()

project(cmdlime VERSION 1.0.0 DESCRIPTION "C++17 command line parsing library")

add_library(cmdlime INTERFACE)
target_compile_features(cmdlime INTERFACE cxx_std_17)
target_include_directories(
    cmdlime
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(ENABLE_TESTS "Enable tests" OFF)
option(ENABLE_LIB_TESTS "Enable tests when cmdlime is bundled within other project" OFF)
if ((ENABLE_TESTS AND NOT CMDLIME_SUBPROJECT) OR (ENABLE_LIB_TESTS AND CMDLIME_SUBPROJECT))
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
if(NOT CMDLIME_SUBPROJECT)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/cmdlime DESTINATION include)
endif()
