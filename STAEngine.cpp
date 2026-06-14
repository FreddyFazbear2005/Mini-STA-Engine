#include "STAEngine.hpp"
#include "Node.hpp"
#include <algorithm>
#include <queue>
#include <stdexcept>

STAEngine::STAEngine(TimingGraph &g) : graph(g) {}

std::vector<NodeID> STAEngine::topologicalSort() {

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());

  std::vector<int> inDegree(numberOfNodes, 0);

  std::queue<NodeID> nodeQueue;

  std::vector<NodeID> result;

  for (NodeID i = 0; i < numberOfNodes; i++) {
    for (const auto &neighbour : graph.getNode(i).fanout) {
      inDegree[neighbour]++;
    }
  }

  for (NodeID i = 0; i < numberOfNodes; i++) {
    if (inDegree[i] == 0) {
      nodeQueue.push(i);
    }
  }

  while (nodeQueue.empty() == false) {
    NodeID currentNode = nodeQueue.front();
    nodeQueue.pop();
    result.push_back(currentNode);
    for (const auto &neighbour : graph.getNode(currentNode).fanout) {
      inDegree[neighbour]--;
      if (inDegree[neighbour] == 0) {
        nodeQueue.push(neighbour);
      }
    }
  }
  if (result.size() != graph.size()) {
    throw std::runtime_error("Combinational loop detected");
  }
  return result;
}

void STAEngine::computeArrivalTimes() {
  std::vector<NodeID> sortedNodes = topologicalSort();

  for (NodeID currentNodeID : sortedNodes) {
    Node &currentNode = graph.getNode(currentNodeID);

    double maxArrival = 0.0;

    for (NodeID predecessorNodeID : currentNode.fanin) {
      const Node &predecessorNode = graph.getNode(predecessorNodeID);
      if (predecessorNode.timing.arrival > maxArrival) {
        maxArrival = predecessorNode.timing.arrival;
        currentNode.criticalPredecessor = predecessorNodeID;
      }
    }

    currentNode.timing.arrival = maxArrival + currentNode.cellDelay;
  }
}

void STAEngine::computeRequiredTimes(double clockPeriod) {
  std::vector<NodeID> sortedNodes = topologicalSort();

  // At the end points the nodes will have a requirement of clockPeriod

  for (NodeID currentNodeID : sortedNodes) {
    Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.fanout.empty() == true) {
      currentNode.timing.required = clockPeriod;
    }
  }

  for (auto currentNodeIterator = sortedNodes.rbegin();
       currentNodeIterator != sortedNodes.rend(); ++currentNodeIterator) {
    Node &currentNode = graph.getNode(*currentNodeIterator);
    for (NodeID successorNodeID : currentNode.fanout) {
      const Node &successorNode = graph.getNode(successorNodeID);
      currentNode.timing.required =
          std::min(currentNode.timing.required,
                   successorNode.timing.required - successorNode.cellDelay);
    }
  }
}

void STAEngine::computeSlack() {
  for (NodeID currentNodeID = 0; currentNodeID < graph.size();
       currentNodeID++) {
    Node &currentNode = graph.getNode(currentNodeID);
    currentNode.timing.slack =
        currentNode.timing.required - currentNode.timing.arrival;
  }
}
