cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    GMQ-Headers
    LANGUAGES CXX
)

enable_testing()
add_subdirectory(3rdlibs/glfw)
add_subdirectory(3rdlibs/glad)
add_subdirectory(bins)
