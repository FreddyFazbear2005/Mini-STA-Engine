#include "STAEngine.hpp"
#include <iostream>

int main() {
  TimingGraph graph;

  NodeID ff1Q = graph.addNode("FF1_Q", NodeType::flipFlopQ);
  NodeID u1 = graph.addNode("U1", NodeType::gate);
  NodeID ff2D = graph.addNode("FF2_D", NodeType::flipFlopD);

  graph.getNode(ff1Q).clockToQ = 0.5;

  graph.getNode(u1).maxCellDelay = 5.0;
  graph.getNode(u1).minCellDelay = 1.0;

  graph.getNode(ff2D).setupTime = 0.5;
  graph.getNode(ff2D).holdTime = 0.2;

  graph.addEdge(ff1Q, u1);
  graph.addEdge(u1, ff2D);

  STAEngine sta(graph);

  sta.run(10);

  sta.displayTimingReport();

  std::cout << "\nSETUP ANALYSIS\n";
  sta.displayCriticalPath();

  std::cout << "\nHOLD ANALYSIS\n";
  sta.displayHoldCriticalPath();

  return 0;
}
