#include "graph.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <stack>

void Graph::addArc(Arc arc) {
  assert(arc.To && "Arcs to the root vertex are prohibited");
  std::size_t MaxNode{std::max(arc.From, arc.To)};
  if (MaxNode >= Data.size())
    Data.resize(MaxNode + 1);
  Data[arc.From].insert(arc.To);
}

Graph::Graph(const std::vector<Arc> &arcs) {
  Data.emplace_back(NodeSet{});
  for (const auto arc : arcs)
    addArc(arc);
}

Graph::Graph(std::initializer_list<Arc> Arcs) {
  Data.emplace_back(NodeSet{});
  for (auto Arc : Arcs)
    addArc(Arc);
}

const Graph::NodeSet &Graph::successors(std::size_t vertex) const {
  assert(vertex < size() && "The vertex is not in the graph");
  return Data[vertex];
}

std::vector<std::size_t> Graph::DFSOrder() const {
  std::vector<std::size_t> Result{};
  Result.reserve(size());
  std::stack<std::size_t> Stack{};
  std::unordered_set<std::size_t> Visited{};
  Stack.push(0);
  while (!Stack.empty()) {
    auto CurrentVertex{Stack.top()};
    Stack.pop();
    if (Visited.count(CurrentVertex))
      continue;
    Result.push_back(CurrentVertex);
    Visited.insert(CurrentVertex);
    const auto &AdjacentToCurrent = successors(CurrentVertex);
    for (auto It = std::cbegin(AdjacentToCurrent),
              E = std::cend(AdjacentToCurrent);
         It != E; ++It)
      Stack.push(*It);
  }
  return Result;
}

void Graph::dump() const {
  for (int I = 0, E = size(); I < E; ++I) {
    const auto &AdjacentToCurrent = successors(I);
    for (auto Dest : AdjacentToCurrent)
      std::cout << I << " -> " << Dest << "\n";
  }
}
