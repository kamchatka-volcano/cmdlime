cmake_minimum_required(VERSION 2.6)
 
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}, ..)

set(SRC
    test_default_format.cpp 
)

add_executable(test_cmdlime ${SRC})
add_test(NAME test_cmdlime COMMAND test_cmdlime)
target_include_directories(test_cmdlime PRIVATE ../include/cmdlime)
target_link_libraries(test_fcgi gtest gmock gtest_main pthread)
