cmake_minimum_required(VERSION 3.9)
project(disparity_cpu)

set(CMAKE_CXX_STANDARD 14)
include_directories($(PROJECT_DIRECTORY)/incl)
include_directories($(PROJECT_DIRECTORY)/thirdparty)
set(SOURCE_FILES src/main.cpp src/PixelUtils.cpp src/Logger.cpp thirdparty/lodepng.cpp src/Disparity.cpp src/CliOptions.cpp)
add_executable(disparity_cpu ${SOURCE_FILES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(disparity_cpu Threads::Threads)