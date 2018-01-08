#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_set>
#include <vector>

// \brief Arc of a Graph.
struct Arc {
  std::size_t From;
  std::size_t To;
};

// \brief Directed graph with pointed root.
class Graph {
public:
  using NodeSet = std::unordered_set<std::size_t>;

  // \brief Add \p arc to the graph.
  // If the arc already exists does nothing.
  // Create new vertexes if necessary.
  // Precondition: arc must not be to the root vertex (#0).
  void addArc(Arc arc);

  // \brief Number of roots in the graph.
  auto size() const { return Data.size(); }

  // \brief Successors of a node pointed by \p index.
  const NodeSet &successors(std::size_t vertex) const;

  // \brief Graph nodes listed in DFS order.
  std::vector<std::size_t> DFSOrder() const;

  /// \return If the graph has \p arc
  bool hasArc(Arc A) const {
    if (A.From >= Data.size())
      return false;
    return Data[A.From].count(A.To);
  }

  // \brief Print the graph to stdout.
  void dump() const;

  Graph(const std::vector<Arc> &arcs);
  Graph(std::initializer_list<Arc> Arcs);

  static constexpr size_t Root = 0;

private:
  // \brief Adjacency list of the graph.
  std::vector<NodeSet> Data{};
};

#endif // GRAPH_H
