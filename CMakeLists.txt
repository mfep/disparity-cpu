cmake_minimum_required(VERSION 3.9)
project(cpu_golden)

set(CMAKE_CXX_STANDARD 14)

add_executable(cpu_golden main.cpp thirdparty/lodepng.cpp)