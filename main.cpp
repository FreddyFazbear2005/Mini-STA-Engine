#include "NetlistParser.hpp"
#include "STAEngine.hpp"
#include "TimingGraph.hpp"

int main() {
  TimingGraph graph;
  NetListParser parser(graph);
  parser.parse("STA.txt");

  STAEngine sta(graph);

  sta.run(parser.getClockPeriod());

  sta.displayTimingReport();
  sta.displaySetupCriticalPaths(5);
  sta.displayHoldCriticalPaths(5);
}
