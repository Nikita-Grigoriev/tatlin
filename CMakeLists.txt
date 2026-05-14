cmake_minimum_required(VERSION 3.20)
project(tatlin CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)
