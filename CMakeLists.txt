cmake_minimum_required(VERSION 3.20)
project(soclib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER clang++)

# Core library
add_library(soclib STATIC
    src/str.cpp
    src/file.cpp
    src/os.cpp
    src/console.cpp
)
target_include_directories(soclib PUBLIC include)
target_compile_options(soclib PRIVATE -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic)

# Modular testing via CTest
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()