cmake_minimum_required(VERSION 3.8)
project(compiler_algo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -Werror")
include_directories(include)

add_subdirectory(src)
