#include "STAEngine.hpp"

int main() {
  TimingGraph graph;

  NodeID ff1Q = graph.addNode("FF1_Q", NodeType::flipFlopQ);
  NodeID ff2Q = graph.addNode("FF2_Q", NodeType::flipFlopQ);

  NodeID u1 = graph.addNode("U1");
  NodeID u2 = graph.addNode("U2");

  NodeID ff3D = graph.addNode("FF3_D", NodeType::flipFlopD);

  graph.addEdge(ff1Q, u1);
  graph.addEdge(u1, ff3D);
  graph.addEdge(ff2Q, u2);
  graph.addEdge(u2, ff3D);

  graph.getNode(ff1Q).clockToQ = 0.5;
  graph.getNode(ff2Q).clockToQ = 0.5;

  graph.getNode(u1).cellDelay = 5.0;
  graph.getNode(u2).cellDelay = 5.0;

  graph.getNode(ff3D).setupTime = 0.5;

  STAEngine sta(graph);

  sta.run(10.0);

  sta.displayTimingReport();
  sta.displayCriticalPath();

  return 0;
}
