cmake_minimum_required(VERSION 3.25)
project(gits CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(gits main.cpp)

# Point directly to the include directory containing rang.hpp
target_include_directories(gits PRIVATE external/rang/include)
