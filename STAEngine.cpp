#include "STAEngine.hpp"
#include "Node.hpp"
#include "TimingGraph.hpp"
#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>

STAEngine::STAEngine(TimingGraph &g) : graph(g) {}

void STAEngine::reset() {
  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID i = 0; i < numberOfNodes; ++i) {
    Node &node = graph.getNode(i);
    node.timing.maxArrival = 0;
    node.setupCriticalPredecessor = -1;
    node.timing.required = std::numeric_limits<double>::infinity();
    node.timing.setupSlack = 0;
    node.timing.holdSlack = 0;
    node.holdCriticalPredecessor = -1;
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

void STAEngine::computeMaxArrivalTimes() {
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
      currentNode.timing.maxArrival = maxArrival + currentNode.maxCellDelay;
    }
  }
}

void STAEngine::computeMinArrivalTimes() {
  std::vector<NodeID> sortedNodes = topologicalSort();

  for (NodeID currentNodeID : sortedNodes) {
    Node &currentNode = graph.getNode(currentNodeID);

    if (currentNode.fanin.empty() && currentNode.type == NodeType::gate) {
      throw std::runtime_error("Gate has no fanin");
    }

    double minArrival = std::numeric_limits<double>::infinity();

    for (NodeID predecessorNodeID : currentNode.fanin) {
      const Node &predecessorNode = graph.getNode(predecessorNodeID);
      if (predecessorNode.timing.minArrival <= minArrival) {
        minArrival = predecessorNode.timing.minArrival;
        currentNode.holdCriticalPredecessor = predecessorNodeID;
      }
    }

    if (currentNode.type ==
        NodeType::flipFlopQ) { // FF_Q is treated as a timing start point, graph
                               // must be constructed to reflect this Arrival is
                               // initialized to clock to Q delay and does not
                               // depend on predecessor nodes.
      currentNode.timing.minArrival = currentNode.clockToQ;

    } else if (currentNode.type == NodeType::primaryInput) {
      currentNode.timing.minArrival = 0.0;
    } else {
      currentNode.timing.minArrival = minArrival + currentNode.minCellDelay;
    }
  }
}

void STAEngine::computeMaxRequiredTimes(double clockPeriod) {
  std::vector<NodeID> sortedNodes = topologicalSort();

  // At the end points the nodes will have a requirement of clockPeriod

  for (NodeID currentNodeID : sortedNodes) {
    Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::primaryOutput) {
      currentNode.timing.required = clockPeriod;
    } else if (currentNode.type == NodeType::flipFlopD) {
      currentNode.timing.required = clockPeriod - currentNode.setupTime;
    }
  }

  for (auto currentNodeIterator = sortedNodes.rbegin();
       currentNodeIterator != sortedNodes.rend(); ++currentNodeIterator) {
    Node &currentNode = graph.getNode(*currentNodeIterator);
    for (NodeID successorNodeID : currentNode.fanout) {
      const Node &successorNode = graph.getNode(successorNodeID);
      currentNode.timing.required =
          std::min(currentNode.timing.required,
                   successorNode.timing.required - successorNode.maxCellDelay);
    }
  }
}

void STAEngine::computeSetupSlack() {
  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    Node &currentNode = graph.getNode(currentNodeID);
    currentNode.timing.setupSlack =
        currentNode.timing.required - currentNode.timing.maxArrival;
  }
}

void STAEngine::computeHoldSlack() {
  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());

  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::flipFlopD) {
      currentNode.timing.holdSlack =
          currentNode.timing.minArrival - currentNode.holdTime;
    }
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

void STAEngine::displaySetupCriticalPaths(size_t numberofPaths) {
  constexpr int REPORT_WIDTH = 70;
  constexpr int PRECISION = 2;

  std::cout << std::fixed << std::setprecision(PRECISION);

  if (graph.size() == 0) {
    std::cout << "The graph is empty" << std::endl;
    return;
  }

  std::vector<TimingPath> setupCriticalPaths;

  NodeID worstEndNodeID = -1;

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    const Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::flipFlopD ||
        currentNode.type == NodeType::primaryOutput) {
      setupCriticalPaths.push_back(
          TimingPath{currentNodeID, currentNode.timing.setupSlack});
    }
  }

  if (setupCriticalPaths.empty() == true) {
    std::cout << "No setup endpoints found.\n";
    return;
  }

  std::sort(setupCriticalPaths.begin(), setupCriticalPaths.end(),
            [](const TimingPath &a, const TimingPath &b) {
              return a.setupSlack < b.setupSlack;
            });
  size_t pathsToDisplay = std::min(numberofPaths, setupCriticalPaths.size());
  std::cout << "\nSETUP CRITICAL PATH REPORT\n";
  std::cout << "Endpoints analyzed : " << setupCriticalPaths.size() << "\n";
  std::cout << "Displaying " << pathsToDisplay << " paths(s)" << "\n\n";

  for (size_t pathIndex = 0; pathIndex < pathsToDisplay; pathIndex++) {
    std::vector<NodeID> criticalPath;
    worstEndNodeID = setupCriticalPaths[pathIndex].endNodeID;
    while (worstEndNodeID != -1) {
      criticalPath.push_back(worstEndNodeID);
      worstEndNodeID = graph.getNode(worstEndNodeID).setupCriticalPredecessor;
    }
    std::reverse(criticalPath.begin(), criticalPath.end());
    std::cout << std::string(REPORT_WIDTH, '-') << '\n';
    std::cout << "Path " << pathIndex + 1 << "\n";
    std::cout << "Slack : " << setupCriticalPaths[pathIndex].setupSlack << " ns"
              << "\n";
    std::cout << "Endpoint : "
              << graph.getNode(setupCriticalPaths[pathIndex].endNodeID).name
              << "\n";
    std::cout << "Path: \n";
    for (size_t i = 0; i < criticalPath.size(); ++i) {
      std::cout << graph.getNode(criticalPath[i]).name;
      if (i + 1 != criticalPath.size()) {
        std::cout << " -> ";
      }
    }
    std::cout << std::endl;
  }
}

void STAEngine::displayHoldCriticalPath() {
  if (graph.size() == 0) {
    std::cout << "The graph is empty" << std::endl;
    return;
  }
  NodeID worstEndNodeID = -1;
  double worstHoldSlack = std::numeric_limits<double>::infinity();

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    const Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::flipFlopD &&
        currentNode.timing.holdSlack < worstHoldSlack) {
      worstHoldSlack = currentNode.timing.holdSlack;
      worstEndNodeID = currentNodeID;
    }
  }

  if (worstEndNodeID == -1) {
    std::cout << "No hold endpoints found\n";
    return;
  }

  std::vector<NodeID> criticalPath;
  std::cout << "Worst Hold Slack : " << worstHoldSlack << " ns\n" << std::endl;
  if (worstHoldSlack < 0.0) {
    std::cout << "HOLD VIOLATION\n\n";
  }
  while (worstEndNodeID != -1) {
    criticalPath.push_back(worstEndNodeID);
    worstEndNodeID = graph.getNode(worstEndNodeID).holdCriticalPredecessor;
  }
  std::reverse(criticalPath.begin(), criticalPath.end());
  std::cout << "\n Worst Hold Path:\n";
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
  // Setup analysis
  computeMaxArrivalTimes();
  computeMaxRequiredTimes(clockPeriod);
  computeSetupSlack();

  // Hold analysis
  computeMinArrivalTimes();
  computeHoldSlack();
}

void STAEngine::displayTimingReport() {
  constexpr int COLUMN_WIDTH = 15;
  constexpr int PRECISION = 2;

  std::cout << std::fixed << std::setprecision(PRECISION);

  std::cout << std::string(6 * COLUMN_WIDTH, '-') << '\n';
  std::cout << std::left << std::setw(COLUMN_WIDTH) << "Node"
            << std::setw(COLUMN_WIDTH) << "MaxArrival"
            << std::setw(COLUMN_WIDTH) << "Required" << std::setw(COLUMN_WIDTH)
            << "SetupSlack" << std::setw(COLUMN_WIDTH) << "minArrival"
            << std::setw(COLUMN_WIDTH) << "HoldSlack" << "\n";

  std::cout << std::string(6 * COLUMN_WIDTH, '-') << '\n';

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeId = 0; currentNodeId < numberOfNodes;
       currentNodeId++) {
    const Node &currentNode = graph.getNode(currentNodeId);
    std::cout << std::left << std::setw(COLUMN_WIDTH) << currentNode.name
              << std::setw(COLUMN_WIDTH) << currentNode.timing.maxArrival
              << std::setw(COLUMN_WIDTH) << currentNode.timing.required
              << std::setw(COLUMN_WIDTH) << currentNode.timing.setupSlack
              << std::setw(COLUMN_WIDTH) << currentNode.timing.minArrival
              << std::setw(COLUMN_WIDTH) << currentNode.timing.holdSlack
              << '\n';
  }
  std::cout << std::string(6 * COLUMN_WIDTH, '-') << '\n';
}
