#ifndef DOMINANCE_H
#define DOMINANCE_H
#include "graph.h"
#include <unordered_map>

namespace wyrm {

using DominatorMap = std::unordered_map<size_t, Graph::NodeSet>;
DominatorMap dominators_slow(const Graph &CFG);

} // namespace wyrm

#endif
