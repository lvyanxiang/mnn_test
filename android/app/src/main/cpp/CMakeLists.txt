cmake_minimum_required(VERSION 3.13.0)

project("mnn-test" CXX)

set(CMAKE_CXX_STANDARD 17)

# Build MnnTestModule.cpp
add_library(
    mnn-test
    SHARED
    MnnTestModule.cpp
)

# Find JNI library
find_library(log-lib log)

# Link libraries
target_link_libraries(
    mnn-test
    ${log-lib}
)
