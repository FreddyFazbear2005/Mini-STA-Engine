#pragma once
#include <limits>
#include <string>
#include <vector>

using NodeID = int;
using EdgeID = int;

enum class NodeType { primaryInput, primaryOutput, gate, flipFlopD, flipFlopQ };

struct TimingInfo {
  double maxArrival = 0.0;
  double required = std::numeric_limits<double>::infinity();
  double setupSlack = 0.0;

  double minArrival = std::numeric_limits<double>::infinity();
  double holdSlack = 0.0;
};

struct Node {
  NodeID id;
  std::string name;
  double maxCellDelay = 0.0;
  double minCellDelay = 0.0;
  double inputDelay = 0.0;
  double outputDelay = 0.0;

  NodeType type = NodeType::gate;
  std::string cellType;

  // std::vector<NodeID> fanin;
  // std::vector<NodeID> fanout;

  std::vector<EdgeID> incomingEdges;
  std::vector<EdgeID> outgoingEdges;

  NodeID setupCriticalPredecessor = -1;
  NodeID holdCriticalPredecessor = -1;

  TimingInfo timing;

  double setupTime = 0.0;
  double holdTime = 0.0;
  double clockToQ = 0.0;

  bool hasSetupTime = false;
  bool hasHoldTime = false;
  bool hasClockToQ = false;
  bool hasInputDelay = false;
  bool hasOutputDelay = false;

  Node(NodeID nodeID, const std::string &nodeName,
       NodeType nodeType = NodeType::gate, const std::string &cellType = "",
       double maxCellDelay = 0.0, double minCellDelay = 0.0)
      : id(nodeID), name(nodeName), maxCellDelay(maxCellDelay),
        minCellDelay(minCellDelay), type(nodeType), cellType(cellType) {}
};
