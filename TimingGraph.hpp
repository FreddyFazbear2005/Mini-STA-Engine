#pragma once

#include "Edge.hpp"
#include "Node.hpp"
#include <cstddef>
#include <vector>

struct TimingPath {
  NodeID endNodeID;
  double setupSlack;
  double holdSlack;
};

class TimingGraph {
private:
  std::vector<Node> nodes;
  std::vector<Edge> edges;

public:
  NodeID addNode(const std::string &name, NodeType type = NodeType::gate,
                 const std::string &cellType = "", double maxCellDelay = 0.0,
                 double minCellDelay = 0.0);

  void addEdge(NodeID src, NodeID dst, double maxDelay = 0.0,
               double minDelay = 0.0);

  Node &getNode(NodeID id);

  const Node &getNode(NodeID id) const;

  const std::vector<Node> &getNodes() const;

  Edge &getEdge(EdgeID id);

  const Edge &getEdge(EdgeID id) const;

  const std::vector<Edge> &getEdges() const;

  size_t size() const;
};
