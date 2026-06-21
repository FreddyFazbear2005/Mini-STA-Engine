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
  void computeMinArrivalTimes();
  void computeMaxArrivalTimes();
  void computeMaxRequiredTimes(double clockPeriod);
  void computeSetupSlack();
  void computeHoldSlack();
  void displayCriticalPath();
  void displayHoldCriticalPath();
  void run(double clockPeriod);
  void displayTimingReport();
};
