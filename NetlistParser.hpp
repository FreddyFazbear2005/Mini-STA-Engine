#include "TimingGraph.hpp"
#include <sstream>
#include <unordered_map>
class NetListParser {
public:
  explicit NetListParser(TimingGraph &graph);
  void parse(const std::string &filename);

private:
  TimingGraph &graph;
  std::unordered_map<std::string, NodeID> nodeMap;
  void parseNode(std::stringstream &ss);
  void parseEdge(std::stringstream &ss);
  void parseDelay(std::stringstream &ss);
  void parseClockToQ(std::stringstream &ss);
};
