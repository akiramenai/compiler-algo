add_executable(compiler_algo
  graph.cpp)

add_executable(gviz
  main.cpp)

find_package(Boost)

target_include_directories(gviz
  PUBLIC
  ${Boost_INCLUDE_DIRS})

target_link_libraries(gviz
  ${Boost_LIBRARY_DIRS})
