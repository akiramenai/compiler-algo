#ifndef DOMINANCE_H
#define DOMINANCE_H
#include "graph.h"
#include <unordered_map>

namespace wyrm {

using DominatorMap = std::unordered_map<size_t, Graph::NodeSet>;
/// \brief Implement straightforwad algorithm of dominators search in \par CFG.
/// The algorithm is described in Muchnick 7.3 (p. 181).
/// \return Hash table which maps a node to its dominators.
DominatorMap dominators_slow(const Graph &CFG);
/// \brief Find immediete dominators.
/// \return Dominator tree.
Graph buildDominatorTree(const Graph &CFG);

} // namespace wyrm

#endif
