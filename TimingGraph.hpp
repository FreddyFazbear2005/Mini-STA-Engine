#pragma once

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

public:
  NodeID addNode(const std::string &name, NodeType type = NodeType::gate);

  void addEdge(NodeID src, NodeID dst);

  Node &getNode(NodeID id);

  const Node &getNode(NodeID id) const;

  const std::vector<Node> &getNodes() const;

  size_t size() const;
};
