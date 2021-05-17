cmake_minimum_required(VERSION 2.6)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}, ..)

set(SRC
    test_simple_format.cpp 
    test_gnu_format.cpp 
    test_posix_format.cpp
    test_x11_format.cpp
    test_nameutils.cpp 
)

add_executable(test_cmdlime ${SRC})
add_test(NAME test_cmdlime COMMAND test_cmdlime)
target_include_directories(test_cmdlime PRIVATE ../include)
target_link_libraries(test_cmdlime gtest gmock gtest_main pthread)
