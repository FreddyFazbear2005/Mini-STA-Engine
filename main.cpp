#include "STAEngine.hpp"

int main() {
  TimingGraph graph;

  auto A = graph.addNode("A", NodeType::primaryInput);
  auto B = graph.addNode("B", NodeType::primaryInput);

  auto U1 = graph.addNode("U1");
  auto U2 = graph.addNode("U2");
  auto U3 = graph.addNode("U3");

  auto OUT = graph.addNode("OUT", NodeType::primaryOutput);

  graph.addEdge(A, U1);
  graph.addEdge(U1, U2);
  graph.addEdge(U2, OUT);

  graph.addEdge(B, U3);
  graph.addEdge(U3, OUT);

  graph.getNode(U1).cellDelay = 3.0;
  graph.getNode(U2).cellDelay = 2.0;
  graph.getNode(U3).cellDelay = 6.0;

  STAEngine sta(graph);

  sta.run(10);

  sta.displayTimingReport();
  sta.displayCriticalPath();
}
