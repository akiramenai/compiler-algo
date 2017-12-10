#include "Analysis/dominance.h"
#include "graph.h"
#include <algorithm>

namespace wyrm {

static bool updateDominators(DominatorMap &DomMap, size_t Predecessor,
                             const Graph &CFG) {
  bool IsChanged{};
  for (auto Node : CFG.successors(Predecessor)) {
    Graph::NodeSet NS{DomMap[Predecessor]};
    NS.insert(Node);
    for (auto It = std::begin(DomMap[Node]); It != std::end(DomMap[Node]);) {
      if (NS.count(*It) == 0u) {
        It = DomMap[Node].erase(It);
        IsChanged = true;
      } else {
        ++It;
      }
    }
  }
  return IsChanged;
}

DominatorMap dominators_slow(const Graph &CFG) {
  std::vector<size_t> Nodes{CFG.DFSOrder()};
  Graph::NodeSet U{std::begin(Nodes), std::end(Nodes)};
  DominatorMap DomMap;
  DomMap[0] = {0};
  for (int I = 1, E = Nodes.size(); I < E; ++I)
    DomMap[I] = U;
  bool IsChanged{true};
  while (IsChanged) {
    IsChanged = false;
    for (auto Node : Nodes)
      IsChanged |= updateDominators(DomMap, Node, CFG);
  }
  return DomMap;
}
} // namespace wyrm
