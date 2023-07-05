cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    GMQ-Headers
    LANGUAGES CXX
)

add_subdirectory(3rdlibs/sol)
add_subdirectory(3rdlibs/lua)

enable_testing()
add_subdirectory(bins)
