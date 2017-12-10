#include "graph.h"
#include "Analysis/dominance.h"
#include "gtest/gtest.h"
#include <algorithm>

std::vector<Graph> Graphs{
    {{0, 1}, {0, 2}, {1, 2}},
    {{0, 1}, {1, 2}, {1, 3}, {2, 7}, {3, 4}, {4, 5}, {4, 6}, {5, 7}, {6, 4}}};

static bool areEqual(const std::vector<std::size_t> &v1,
                     const std::vector<std::size_t> &v2) {
  return std::is_permutation(std::begin(v1), std::end(v1), std::begin(v2));
}

TEST(DFSOrder, Graph) {
  const Graph &IfG{Graphs[0]};
  EXPECT_TRUE(areEqual(IfG.DFSOrder(), {0, 1, 2}));
}

TEST(DominatorsAreCalculatedForAllNodes, Dominance) {
  for (const auto &Graph : Graphs) {
    auto DomMap = wyrm::dominators_slow(Graph);
    EXPECT_EQ(DomMap.size(), Graph.size());
  }
}

TEST(AnyNodeDominatesItself, Dominance) {
  for (const auto &Graph : Graphs) {
    auto DomMap = wyrm::dominators_slow(Graph);
    for (auto v : Graph.DFSOrder()) {
      EXPECT_EQ(DomMap[v].count(v), 1u);
    }
  }
}

TEST(Dominators1, Dominance) {
  Graph &IfG{Graphs[0]};
  auto DomMap = wyrm::dominators_slow(IfG);
  EXPECT_EQ(DomMap[0].size(), 1u);
  EXPECT_EQ(DomMap[1].size(), 2u);
  EXPECT_EQ(DomMap[2].size(), 2u);
  EXPECT_EQ(DomMap[1].count(0), 1u);
  EXPECT_EQ(DomMap[2].count(0), 1u);
}

TEST(Dominators2, Dominance) {
  Graph &G{Graphs[1]};
  auto DomMap = wyrm::dominators_slow(G);
  EXPECT_EQ(DomMap[0].size(), 1u);
  EXPECT_EQ(DomMap[1].size(), 2u);
  EXPECT_EQ(DomMap[2].size(), 3u);
  EXPECT_EQ(DomMap[3].size(), 3u);
  EXPECT_EQ(DomMap[4].size(), 4u);
  EXPECT_EQ(DomMap[5].size(), 5u);
  EXPECT_EQ(DomMap[6].size(), 5u);
  EXPECT_EQ(DomMap[7].size(), 3u);
  auto CHECK_NODE = [&DomMap](unsigned V, std::vector<unsigned> &&Vs) {
    for (unsigned Expected : Vs)
      EXPECT_EQ(1u, DomMap[V].count(Expected));
  };
  CHECK_NODE(1, {0, 1});
  CHECK_NODE(2, {0, 1, 2});
  CHECK_NODE(3, {0, 1, 3});
  CHECK_NODE(4, {0, 1, 3, 4});
  CHECK_NODE(5, {0, 1, 3, 4, 5});
  CHECK_NODE(6, {0, 1, 3, 4, 6});
  CHECK_NODE(7, {0, 1, 7});
}
