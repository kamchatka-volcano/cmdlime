cmake_minimum_required(VERSION 3.18)
project(test_cmdlime)

set(SRC
    test_simple_format.cpp
    test_gnu_format.cpp
    test_posix_format.cpp
    test_x11_format.cpp
    test_validator.cpp
    test_nameutils.cpp
)

SealLake_GoogleTest(
        SOURCES ${SRC}
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES cmdlime::cmdlime
)
