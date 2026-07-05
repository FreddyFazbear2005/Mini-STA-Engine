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
  void computeMaxRequiredTimes(double clockPeriod, double clockUncertainty);
  void computeSetupSlack();
  void computeHoldSlack(double clockUncertainty);
  void displaySetupCriticalPaths(size_t numberofPaths);
  void displayHoldCriticalPaths(size_t numberofPaths);
  void run(double clockPeriod, double clockUncertainty = 0.0);
  void displayTimingReport();
};
