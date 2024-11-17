cmake_minimum_required(VERSION 3.2)
project(mini-redis)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main ${SOURCE_FILES})
