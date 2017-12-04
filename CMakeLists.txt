cmake_minimum_required(VERSION 3.9.3)
project(compiler_algo CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -Werror -Wextra -g")

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
