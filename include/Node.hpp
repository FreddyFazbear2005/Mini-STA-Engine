#include <limits>
#include <string>
#include <vector>

enum class NodeType { primaryInput, primaryOutput, gate, flipFlopD, flipFlopQ };

struct TimingInfo {
  double arrival = 0.0;
  double required = std::numeric_limits<double>::infinity();
  double slack = 0.0;
};

struct Node {
  int id;
  std::string name;
  double cellDelay = 0.0;

  NodeType type = NodeType::gate;
  std::string cellType;

  std::vector<int> fanin;
  std::vector<int> fanout;

  int criticalPredecessor = -1;

  TimingInfo timing;

  Node(int nodeID, const std::string &nodeName) : id(nodeID), name(nodeName) {}
};
