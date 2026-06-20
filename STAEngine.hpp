#pragma once

#include "TimingGraph.hpp"
#include <vector>

class STAEngine {
private:
  TimingGraph &graph;

public:
  explicit STAEngine(TimingGraph &g);

  void reset();
  std::vector<NodeID> topologicalSort();
  void computeArrivalTimes();
  void computeRequiredTimes(double clockPeriod);
  void computeSlack();
  void displayCriticalPath();
  void run(double clockPeriod);
  void displayTimingReport();
};
