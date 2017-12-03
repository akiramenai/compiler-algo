#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

int main() {
  using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
  Graph g(3); // Create a graph with 3 vertices.
  boost::add_edge(0, 1, g);
  boost::add_edge(1, 2, g);
  boost::dynamic_properties dp;
  //dp.property("style", get(&StyleProperty::style, g));
  dp.property("node_id", get(boost::vertex_index, g));
  boost::write_graphviz_dp(std::cout, g, dp);
  return 0;
}
