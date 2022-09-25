cmake_minimum_required(VERSION 3.20)

file(GLOB SRC_FILES "*.cpp")
foreach(SRC_FILE ${SRC_FILES})
    SealLake_StringAfterLast(${SRC_FILE} "/" TEST_NAME)
    SealLake_StringBeforeLast(${TEST_NAME} "." TEST_NAME)

    project(${TEST_NAME})
    SealLake_Executable(
        SOURCES ${SRC_FILE}
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES
            cmdlime::cmdlime
    )
endforeach()