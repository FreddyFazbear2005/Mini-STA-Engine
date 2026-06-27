#include "NetlistParser.hpp"
#include "STAEngine.hpp"
#include "TimingGraph.hpp"

int main() {
  TimingGraph graph;
  NetListParser parser(graph);
  parser.parse("STA.txt");

  STAEngine sta(graph);

  sta.run(10);

  sta.displayTimingReport();
  sta.displayCriticalPath();
  sta.displayHoldCriticalPath();
}
