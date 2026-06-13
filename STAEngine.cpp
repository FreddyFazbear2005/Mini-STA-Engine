#include "STAEngine.hpp"
#include "Node.hpp"

STAEngine::STAEngine(TimingGraph &g) : graph(g) {};

std::vector<NodeID> STAEngine::topologicalSort() {
  // Constructing the Adjacency list

  const size_t numberOfNodes = graph.size();

  std::vector<std::vector<NodeID>> Adjacency(
      numberOfNodes, std::vector<NodeID>(numberOfNodes, 0));

  for (NodeID nodeid = 0; nodeid < numberOfNodes; nodeid++) {
  }
}
