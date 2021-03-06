cmake_minimum_required(VERSION 3.9)
project(disparity_cpu)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        src/main.cpp
        inc/PixelUtils.hpp
        src/PixelUtils.cpp
        inc/Logger.hpp
        src/Logger.cpp
        inc/Disparity.hpp
        src/Disparity.cpp
        inc/CliOptions.hpp
        src/CliOptions.cpp
        inc/PixelCalc.hpp
        src/PixelCalc.cpp
        inc/Float8.hpp
        thirdparty/cxxopts.hpp
        thirdparty/doctest.h
        thirdparty/lodepng.h
        thirdparty/lodepng.cpp)

set(CXX_ARCH_FLAG "-mavx")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_ARCH_FLAG}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_ARCH_FLAG}")

add_executable(disparity_cpu ${SOURCE_FILES})
message(project dir : ${PROJECT_SOURCE_DIR})
target_include_directories(disparity_cpu PRIVATE inc thirdparty)

if (${CMAKE_COMPILER_IS_GNUCXX})
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(disparity_cpu Threads::Threads)
endif ()
