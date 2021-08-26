cmake_minimum_required(VERSION 3.18)
project(test_cmdlime)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(CheckPIESupported)
check_pie_supported()

set(SRC
    test_simple_format.cpp 
    test_gnu_format.cpp 
    test_posix_format.cpp
    test_x11_format.cpp
    test_nameutils.cpp 
)

add_executable(test_cmdlime ${SRC})
target_compile_features(test_cmdlime PUBLIC cxx_std_17)
set_target_properties(test_cmdlime PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE OFF)

add_test(NAME test_cmdlime COMMAND test_cmdlime)
target_include_directories(test_cmdlime PRIVATE ../include)
target_link_libraries(test_cmdlime PRIVATE ${GTEST_BOTH_LIBRARIES} Threads::Threads GSL sfun)
