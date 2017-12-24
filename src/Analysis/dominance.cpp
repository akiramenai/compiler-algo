#include "Analysis/dominance.h"
#include "graph.h"
#include <algorithm>

#include <unordered_set>

namespace wyrm {

/// \brief Intersect two unordered sets in place.
/// \return true if \par LHS changed.
template <typename T>
static bool intersectSets(std::unordered_set<T> &LHS,
                          const std::unordered_set<T> &RHS) {
  bool IsChanged{};
  for (auto It = std::begin(LHS); It != std::end(LHS);) {
    if (RHS.count(*It) == 0u) {
      It = LHS.erase(It);
      IsChanged = true;
    } else {
      ++It;
    }
  }
  return IsChanged;
}

template <typename T>
std::unordered_set<T> operator-(std::unordered_set<T> LHS, T Elem) {
  LHS.erase(Elem);
  return LHS;
}

static bool updateDominators(DominatorMap &DomMap, size_t Predecessor,
                             const Graph &CFG) {
  bool IsChanged{};
  for (auto Node : CFG.successors(Predecessor)) {
    Graph::NodeSet NS{DomMap[Predecessor]};
    NS.insert(Node);
    IsChanged |= intersectSets(DomMap[Node], NS);
  }
  return IsChanged;
}

DominatorMap dominators_slow(const Graph &CFG) {
  std::vector<size_t> Nodes{CFG.DFSOrder()};
  Graph::NodeSet U{std::begin(Nodes), std::end(Nodes)};
  DominatorMap DomMap;
  DomMap[0] = {0};
  for (size_t I = 1, E = Nodes.size(); I < E; ++I)
    DomMap[I] = U;
  bool IsChanged{true};
  while (IsChanged) {
    IsChanged = false;
    for (auto Node : Nodes)
      IsChanged |= updateDominators(DomMap, Node, CFG);
  }
  return DomMap;
}

Graph buildDominatorTree(const Graph &CFG) {
  std::vector<size_t> Nodes{CFG.DFSOrder()};
  Graph::NodeSet U{std::begin(Nodes), std::end(Nodes)};
  DominatorMap DomMap{dominators_slow(CFG)};
  Graph Result{};
  for (auto Node : U - Graph::Root)
    for (auto DomNode : DomMap[Node]) {
      if (DomMap[DomNode].size() + 1 == DomMap[Node].size()) {
        Result.addArc({DomNode, Node});
        break;
      }
    }
  return Result;
}

} // namespace wyrm
