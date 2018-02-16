cmake_minimum_required(VERSION 3.9)
project(cpu_golden)

set(CMAKE_CXX_STANDARD 14)
include_directories($(PROJECT_DIRECTORY)/incl)
add_executable(cpu_golden src/main.cpp thirdparty/lodepng.cpp)
