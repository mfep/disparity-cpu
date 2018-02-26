cmake_minimum_required(VERSION 3.9)
project(disparity_cpu)

set(CMAKE_CXX_STANDARD 14)
include_directories($(PROJECT_DIRECTORY)/incl)
add_executable(disparity_cpu src/main.cpp src/PixelUtils.cpp src/Logger.cpp thirdparty/lodepng.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(disparity_cpu Threads::Threads)