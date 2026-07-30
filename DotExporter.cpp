#include "DotExporter.hpp"
#include "Node.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

void DotExporter::exportGraph(const TimingGraph &graph,
                              const std::string &filename) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("Cannot open " + filename);
  }

  if (graph.size() == 0) {
    throw std::runtime_error("The graph is empty, no image can be generated");
  }

  DotExporter::writeHeader(file);
  DotExporter::writeNodes(file, graph);
  DotExporter::writeEdges(file, graph);
  DotExporter::writeFooter(file);
}

void DotExporter::writeHeader(std::ostream &out) {
  out << "digraph TimingGraph {\n";
  out << "    rankdir=LR;\n\n";

  out << "    node [\n";
  out << "         shape=box,\n";
  out << "         style=filled,\n";
  out << "         fillcolor=lightgray\n";
  out << "    ];\n\n";

  out << "    edge [\n";
  out << "         arrowsize=0.8\n";
  out << "    ];\n\n";
}

void DotExporter::writeNodes(std::ostream &out, const TimingGraph &graph) {
  for (const auto &node : graph.getNodes()) {
    out << "    " << node.name << " [\n";
    if (node.type == NodeType::primaryInput ||
        node.type == NodeType::primaryOutput) {
      out << "        shape=ellipse,\n";
    } else if (node.type == NodeType::flipFlopD ||
               node.type == NodeType::flipFlopQ) {
      out << "        shape=doublecircle,\n";
    }
    out << "        label=\"" << node.name;
    if (!node.cellType.empty()) {
      out << "\\n" << node.cellType;
    }
    out << "\"\n";
    out << "    ];\n";
  }
  out << "\n";
}

void DotExporter::writeEdges(std::ostream &out, const TimingGraph &graph) {
  for (const auto &edge : graph.getEdges()) {
    const Node &src = graph.getNode(edge.source);
    const Node &dst = graph.getNode(edge.destination);
    out << "    " << src.name << " -> " << dst.name << ";\n";
  }
}

void DotExporter::writeFooter(std::ostream &out) { out << "}\n"; }

std::vector<CriticalPath> getWorstSetupPaths(const TimingGraph &graph,
                                             size_t count) {
  std::vector<TimingPath> setupCriticalPaths;
  setupCriticalPaths.reserve(graph.size());

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    const Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::flipFlopD ||
        currentNode.type == NodeType::primaryOutput) {
      setupCriticalPaths.push_back(
          TimingPath{currentNodeID, currentNode.timing.setupSlack, 0});
    }
  }
  if (setupCriticalPaths.empty() == true) {
    throw std::runtime_error("No setup paths found");
  }
  std::stable_sort(setupCriticalPaths.begin(), setupCriticalPaths.end(),
                   [](const TimingPath &a, const TimingPath &b) {
                     return a.setupSlack < b.setupSlack;
                   });
  size_t numberOfPathsToDisplay = std::min(count, setupCriticalPaths.size());
  std::vector<CriticalPath> pathsToDisplay(numberOfPathsToDisplay);
  for (size_t pathIndex = 0; pathIndex < numberOfPathsToDisplay; ++pathIndex) {
    CriticalPath &path = pathsToDisplay[pathIndex];
    path.nodes.reserve(graph.size());
    NodeID currentNodeID = setupCriticalPaths[pathIndex].endNodeID;
    while (currentNodeID != -1) {
      path.nodes.push_back(currentNodeID);
      currentNodeID = graph.getNode(currentNodeID).setupCriticalPredecessor;
    }
    std::reverse(path.nodes.begin(), path.nodes.end());
    path.slack = setupCriticalPaths[pathIndex].setupSlack;
    path.startNodeID = path.nodes.front();
    path.endNodeID = path.nodes.back();
    path.delay = graph.getNode(path.endNodeID).timing.maxArrival;
  }
  return pathsToDisplay;
}

std::vector<CriticalPath> getWorstHoldPaths(const TimingGraph &graph,
                                            size_t count) {
  std::vector<TimingPath> holdCriticalPaths;
  holdCriticalPaths.reserve(graph.size());

  const NodeID numberOfNodes = static_cast<NodeID>(graph.size());
  for (NodeID currentNodeID = 0; currentNodeID < numberOfNodes;
       currentNodeID++) {
    const Node &currentNode = graph.getNode(currentNodeID);
    if (currentNode.type == NodeType::flipFlopD) {
      holdCriticalPaths.push_back(
          TimingPath{currentNodeID, 0, currentNode.timing.holdSlack});
    }
  }
  if (holdCriticalPaths.empty() == true) {
    throw std::runtime_error("No hold paths found");
  }
  std::stable_sort(holdCriticalPaths.begin(), holdCriticalPaths.end(),
                   [](const TimingPath &a, const TimingPath &b) {
                     return a.holdSlack < b.holdSlack;
                   });
  size_t numberOfPathsToDisplay = std::min(count, holdCriticalPaths.size());
  std::vector<CriticalPath> pathsToDisplay(numberOfPathsToDisplay);
  for (size_t pathIndex = 0; pathIndex < numberOfPathsToDisplay; ++pathIndex) {
    CriticalPath &path = pathsToDisplay[pathIndex];
    path.nodes.reserve(graph.size());
    NodeID currentNodeID = holdCriticalPaths[pathIndex].endNodeID;
    while (currentNodeID != -1) {
      path.nodes.push_back(currentNodeID);
      currentNodeID = graph.getNode(currentNodeID).holdCriticalPredecessor;
    }
    std::reverse(path.nodes.begin(), path.nodes.end());
    path.slack = holdCriticalPaths[pathIndex].holdSlack;
    path.startNodeID = path.nodes.front();
    path.endNodeID = path.nodes.back();
    path.delay = graph.getNode(path.endNodeID).timing.minArrival;
  }
  return pathsToDisplay;
}
