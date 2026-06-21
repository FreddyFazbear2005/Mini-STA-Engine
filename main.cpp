#include "STAEngine.hpp"
#include <iostream>

int main() {
  TimingGraph graph;

  NodeID ff1Q = graph.addNode("FF1_Q", NodeType::flipFlopQ);
  NodeID ff2D = graph.addNode("FF2_D", NodeType::flipFlopD);

  graph.getNode(ff1Q).clockToQ = 0.05;
  graph.getNode(ff2D).holdTime = 0.10;

  graph.addEdge(ff1Q, ff2D);

  STAEngine sta(graph);
  std::cout << "Running STA\n";
  sta.run(10.0);

  std::cout << "Printing report\n";
  sta.displayTimingReport();

  std::cout << "Printing hold path\n";
  sta.displayHoldCriticalPath();

  std::cout << "Done\n";
  return 0;
}
