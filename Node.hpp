#pragma once
#include <limits>
#include <string>
#include <vector>

using NodeID = int;

enum class NodeType { primaryInput, primaryOutput, gate, flipFlopD, flipFlopQ };

struct TimingInfo {
  double maxArrival = 0.0;
  double maxRequired = std::numeric_limits<double>::infinity();
  double setupSlack = 0.0;
};

struct Node {
  NodeID id;
  std::string name;
  double cellDelay = 0.0;

  NodeType type = NodeType::gate;
  std::string cellType;

  std::vector<NodeID> fanin;
  std::vector<NodeID> fanout;

  NodeID setupCriticalPredecessor = -1;

  TimingInfo timing;

  double setupTime = 0.0;
  double holdTime = 0.0;
  double clockToQ = 0.0;

  Node(NodeID nodeID, const std::string &nodeName,
       NodeType nodeType = NodeType::gate)
      : id(nodeID), name(nodeName), type(nodeType) {}
};
