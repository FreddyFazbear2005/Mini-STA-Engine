#pragma once
#include "TimingGraph.hpp"
#include <sstream>
#include <unordered_map>
class NetListParser {
public:
  explicit NetListParser(TimingGraph &graph);
  void parse(const std::string &filename);
  double getClockPeriod() const;
  double getClockUncertainty() const;

private:
  TimingGraph &graph;
  double clockPeriod = -1;
  double clockUncertainty = 0;
  bool hasClockUncertainty = false;
  std::unordered_map<std::string, NodeID> nodeMap;
  void parseNode(std::stringstream &ss);
  void parseEdge(std::stringstream &ss);
  void parseDelay(std::stringstream &ss);
  void parseClockToQ(std::stringstream &ss);
  void parseSetup(std::stringstream &ss);
  void parseHold(std::stringstream &ss);
  void parseClockPeriod(std::stringstream &ss);
  void parseClockUncertainty(std::stringstream &ss);
  void validate() const;
};
