#include "DotExporter.hpp"
#include "Node.hpp"
#include "TimingGraph.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <unordered_map>

void DotExporter::exportGraph(const TimingGraph &graph,
                              const std::string &filename,
                              const std::vector<CriticalPath> &setupPaths,
                              const std::vector<CriticalPath> &holdPaths) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("Cannot open " + filename);
  }

  if (graph.size() == 0) {
    throw std::runtime_error("The graph is empty, no image can be generated");
  }

  std::unordered_map<EdgeKey, EdgeStyle, EdgeKeyHash> edgeStyles;
  buildEdgeStyles(edgeStyles, setupPaths, holdPaths);
  DotExporter::writeHeader(file);
  DotExporter::writeNodes(file, graph);
  DotExporter::writeEdges(file, graph, edgeStyles);
  DotExporter::writeLegend(file);
  DotExporter::writeFooter(file);
}

void DotExporter::writeHeader(std::ostream &out) {
  out << "digraph TimingGraph {\n";
  out << "    rankdir=LR;\n\n";

  out << "    node [\n";
  out << "         shape=box,\n";
  out << "         style=filled,\n";
  out << "         fillcolor=lightgray,\n";
  out << "         fontname=\"Helvetica\"\n";
  out << "    ];\n\n";

  out << "    edge [\n";
  out << "         arrowsize=0.8,\n";
  out << "         fontname=\"Helvetica\"\n";
  out << "    ];\n\n";
}

void DotExporter::writeNodes(std::ostream &out, const TimingGraph &graph) {
  out << std::fixed << std::setprecision(2);
  for (const Node &node : graph.getNodes()) {
    out << "    \"" << node.name << "\" [\n";
    if (node.type == NodeType::primaryInput ||
        node.type == NodeType::primaryOutput) {
      out << "        shape=ellipse,\n";
    } else if (node.type == NodeType::flipFlopQ) {
      out << "        shape=doublecircle,\n";
    } else if (node.type == NodeType::flipFlopD) {
      out << "        shape=octagon,\n";
    }
    out << "        label=\"" << node.name;
    if (!node.cellType.empty()) {
      out << "\\n" << node.cellType;
    }
    out << "\\nMax Arrival : " << node.timing.maxArrival << " ns";
    out << "\\nMin Arrival : " << node.timing.minArrival << " ns";
    out << "\\nSetup Slack : " << node.timing.setupSlack << " ns";
    out << "\\nHold Slack  : " << node.timing.holdSlack << " ns";
    out << "\"\n";
    out << "    ];\n";
  }
  out << "\n";
}

void DotExporter::writeEdges(
    std::ostream &out, const TimingGraph &graph,
    const std::unordered_map<EdgeKey, EdgeStyle, EdgeKeyHash> &edgeStyles) {
  for (const Edge &edge : graph.getEdges()) {
    const Node &src = graph.getNode(edge.source);
    const Node &dst = graph.getNode(edge.destination);
    auto it = edgeStyles.find({edge.source, edge.destination});
    if (it == edgeStyles.end()) {
      out << "    \"" << src.name << "\" -> \"" << dst.name << "\";\n";
    } else {
      const EdgeStyle &style = it->second;
      out << "    \"" << src.name << "\" -> \"" << dst.name << "\" [\n";
      out << "        color=\"" << style.color << "\",\n ";
      out << "        penwidth=" << style.penWidth << "\n";
      out << "    ];\n";
    }
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

void DotExporter::writeLegend(std::ostream &out) {
  out << "subgraph cluster_legend {\n";
  out << "    label=\"Legend\";\n";
  out << "    fontsize=12;\n";
  out << "    fillcolor=\"#F8F8F8\";\n";
  out << "    style=\"rounded,filled\";\n";
  out << "    margin=12;\n";
  out << "    fontname=\"Helvetica\";\n";

  out << "    node   [style=filled,fillcolor=lightgray];\n";
  out << "    PI_PO  [shape=ellipse,      label=\"Primary I/O\"];\n";
  out << "    G      [shape=box,      label=\"Logic Gate\"];\n";
  out << "    FFQ    [shape=doublecircle,      label=\"Launch FF\"];\n";
  out << "    FFD    [shape=octagon,      label=\"Capture FF\"];\n";

  out << "    Setup1 [shape=none, width=0, height=0, label=\"\"];\n";
  out << "    Setup2 [shape=none, width=0, height=0, label=\"\"];\n";
  out << "    Setup1 -> Setup2 [color=red, penwidth=4];\n";

  out << "    Hold1 [shape=none, width=0, height=0, label=\"\"];\n";
  out << "    Hold2 [shape=none, width=0, height=0, label=\"\"];\n";
  out << "    Hold1 -> Hold2 [color=blue, penwidth=4];\n";

  out << "    SetupLabel [shape=plaintext, label=\"Setup Critical Path\"];\n";
  out << "    HoldLabel  [shape=plaintext, label=\"Hold Critical Path\"];\n\n";

  out << "    Setup2 -> SetupLabel [style=invis];\n";
  out << "    Hold2 -> HoldLabel [style=invis];\n";
  out << "    PI_PO -> G -> FFQ -> FFD [style=invis];\n";
  out << "    { rank=same; PI_PO; G; FFQ; FFD; }\n";
  out << "    { rank=same; Setup1; Setup2; SetupLabel; }\n";
  out << "    { rank=same; Hold1; Hold2; HoldLabel; }\n";

  out << "}\n";
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

void buildEdgeStyles( // Preserves the style of the most critical path
                      // Less critical paths do no over write it
    std::unordered_map<EdgeKey, EdgeStyle, EdgeKeyHash> &edgeStyles,
    const std::vector<CriticalPath> &setupPaths,
    const std::vector<CriticalPath> &holdPaths) {
  const double red_hue = 0.0;
  const double red_saturation = 1.0;
  for (size_t i = 0; i < setupPaths.size(); ++i) {
    const CriticalPath &setUpCriticalPath = setupPaths[i];
    double t =
        static_cast<double>(i) / std::max<size_t>(1, setupPaths.size() - 1);
    double value = 0.3 + 0.7 * (1.0 - t);
    double penWidth = 5 - 3.5 * t;
    std::string color = std::to_string(red_hue) + "," +
                        std::to_string(red_saturation) + "," +
                        std::to_string(value);

    for (size_t j = 0; j + 1 < setUpCriticalPath.nodes.size(); ++j) {
      EdgeKey key{setUpCriticalPath.nodes[j], setUpCriticalPath.nodes[j + 1]};
      if (edgeStyles.find(key) == edgeStyles.end()) {
        edgeStyles[key] = {color, penWidth};
      }
    }
  }
  const double blue_hue = 2.0 / 3.0;
  const double blue_saturation = 0.75;
  for (size_t i = 0; i < holdPaths.size(); ++i) {
    const CriticalPath &holdCriticalPath = holdPaths[i];
    double t =
        static_cast<double>(i) / std::max<size_t>(1, holdPaths.size() - 1);
    double value = 0.45 + 0.55 * (1.0 - t);
    double penWidth = 5 - 3.5 * t;
    std::string color = std::to_string(blue_hue) + "," +
                        std::to_string(blue_saturation) + "," +
                        std::to_string(value);

    for (size_t j = 0; j + 1 < holdCriticalPath.nodes.size(); ++j) {
      EdgeKey key{holdCriticalPath.nodes[j], holdCriticalPath.nodes[j + 1]};
      if (edgeStyles.find(key) == edgeStyles.end()) {
        edgeStyles[key] = {color, penWidth};
      }
    }
  }
}
