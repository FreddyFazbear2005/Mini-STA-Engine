#pragma once
#include <limits>
#include <string>
#include <vector>

using NodeID = int;

enum class NodeType { primaryInput, primaryOutput, gate, flipFlopD, flipFlopQ };

struct TimingInfo {
  double arrival = 0.0;
  double required = std::numeric_limits<double>::infinity();
  double slack = 0.0;
};

struct Node {
  NodeID id;
  std::string name;
  double cellDelay = 0.0;

  NodeType type = NodeType::gate;
  std::string cellType;

  std::vector<NodeID> fanin;
  std::vector<NodeID> fanout;

  NodeID criticalPredecessor = -1;

  TimingInfo timing;

  Node(NodeID nodeID, const std::string &nodeName,
       NodeType nodeType = NodeType::gate)
      : id(nodeID), name(nodeName), type(nodeType) {}
};
