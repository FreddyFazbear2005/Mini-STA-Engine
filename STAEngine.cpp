#include "STAEngine.hpp"
#include "Node.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <queue>
#include <stdexcept>

STAEngine::STAEngine(TimingGraph &g) : graph(g) {}

void STAEngine::reset() {
  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID i = 0; i < numberOfNodes; ++i) {
    Node &node = graph.getNode(i);
    node.timing.maxArrival = 0;
    node.setupCriticalPredecessor = -1;
    node.timing.maxRequired = std::numeric_limits<double>::infinity();
    node.timing.setupSlack = 0;
  }
}

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
      if (predecessorNode.timing.maxArrival >= maxArrival) {
        maxArrival = predecessorNode.timing.maxArrival;
        currentNode.setupCriticalPredecessor = predecessorNodeID;
      }
    }

    if (currentNode.type ==
        NodeType::flipFlopQ) { // FF_Q is treated as a timing start point, graph
                               // must be constructed to reflect this Arrival is
                               // initialized to clock to Q delay and does not
                               // depend on predecessor nodes.
      currentNode.timing.maxArrival = currentNode.clockToQ;
    } else {
      currentNode.timing.maxArrival = maxArrival + currentNode.cellDelay;
    }
  }
}

void STAEngine::computeRequiredTimes(double clockPeriod) {
  std::vector<NodeID> sortedNodes = topologicalSort();

  // At the end points the nodes will have a requirement of clockPeriod

  for (NodeID currentNodeID : sortedNodes) {
    Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::primaryOutput) {
      currentNode.timing.maxRequired = clockPeriod;
    } else if (currentNode.type == NodeType::flipFlopD) {
      currentNode.timing.maxRequired = clockPeriod - currentNode.setupTime;
    }
  }

  for (auto currentNodeIterator = sortedNodes.rbegin();
       currentNodeIterator != sortedNodes.rend(); ++currentNodeIterator) {
    Node &currentNode = graph.getNode(*currentNodeIterator);
    for (NodeID successorNodeID : currentNode.fanout) {
      const Node &successorNode = graph.getNode(successorNodeID);
      currentNode.timing.maxRequired =
          std::min(currentNode.timing.maxRequired,
                   successorNode.timing.maxRequired - successorNode.cellDelay);
    }
  }
}

void STAEngine::computeSlack() {
  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    Node &currentNode = graph.getNode(currentNodeID);
    currentNode.timing.setupSlack =
        currentNode.timing.maxRequired - currentNode.timing.maxArrival;
  }
}

void STAEngine::displayCriticalPath() {

  if (graph.size() == 0) {
    std::cout << "The graph is empty" << std::endl;
    return;
  }
  NodeID worstEndNodeID = -1;
  double worstSlack = std::numeric_limits<double>::infinity();

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    const Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.fanout.empty() &&
        currentNode.timing.setupSlack < worstSlack) {
      worstSlack = currentNode.timing.setupSlack;
      worstEndNodeID = currentNodeID;
    }
  }

  std::vector<NodeID> criticalPath;
  std::cout << "The critical path has a slack of " << worstSlack << " ns\n"
            << std::endl;
  while (worstEndNodeID != -1) {
    criticalPath.push_back(worstEndNodeID);
    worstEndNodeID = graph.getNode(worstEndNodeID).setupCriticalPredecessor;
  }
  std::reverse(criticalPath.begin(), criticalPath.end());
  std::cout << "\n Critical Path:\n";
  for (size_t i = 0; i < criticalPath.size(); ++i) {
    std::cout << graph.getNode(criticalPath[i]).name;
    if (i + 1 != criticalPath.size()) {
      std::cout << " -> ";
    }
  }
  std::cout << std::endl;
}

void STAEngine::run(double clockPeriod) {
  reset();
  computeArrivalTimes();
  computeRequiredTimes(clockPeriod);
  computeSlack();
}

void STAEngine::displayTimingReport() {
  constexpr int COLUMN_WIDTH = 15;
  constexpr int PRECISION = 2;

  std::cout << std::fixed << std::setprecision(PRECISION);

  std::cout << std::string(4 * COLUMN_WIDTH, '-') << '\n';
  std::cout << std::left << std::setw(COLUMN_WIDTH) << "Node"
            << std::setw(COLUMN_WIDTH) << "Arrival" << std::setw(COLUMN_WIDTH)
            << "Required" << std::setw(COLUMN_WIDTH) << "Slack" << '\n';

  std::cout << std::string(4 * COLUMN_WIDTH, '-') << '\n';

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeId = 0; currentNodeId < numberOfNodes;
       currentNodeId++) {
    const Node &currentNode = graph.getNode(currentNodeId);
    std::cout << std::left << std::setw(COLUMN_WIDTH) << currentNode.name
              << std::setw(COLUMN_WIDTH) << currentNode.timing.maxArrival
              << std::setw(COLUMN_WIDTH) << currentNode.timing.maxRequired
              << std::setw(COLUMN_WIDTH) << currentNode.timing.setupSlack
              << '\n';
  }
  std::cout << std::string(4 * COLUMN_WIDTH, '-') << '\n';
}
