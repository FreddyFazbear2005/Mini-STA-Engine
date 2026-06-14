#pragma once

#include "TimingGraph.hpp"
#include <vector>

class STAEngine {
private:
  TimingGraph &graph;

public:
  explicit STAEngine(TimingGraph &g);

  std::vector<NodeID> topologicalSort();
  void computeArrivalTimes();
};
