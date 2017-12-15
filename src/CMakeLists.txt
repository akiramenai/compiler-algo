add_library(graph
  graph.cpp)
add_executable(gviz
  main.cpp)
include_directories(
  ${Boost_INCLUDE_DIRS})

target_link_libraries(gviz
  boost_graph)

add_subdirectory(Analysis)
