cmake_minimum_required(VERSION 3.10)
project(threadPool VERSION 1.0)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(include)
add_executable(main main.cpp threadpool.cpp)

target_link_libraries(main pthread)

