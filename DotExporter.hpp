#pragma once

#include "TimingGraph.hpp"
#include <cstddef>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

struct CriticalPath {
  std::vector<NodeID> nodes;
  double slack;
  double delay;
  NodeID endNodeID;
  NodeID startNodeID;
};

namespace DotExporter {
void exportGraph(const TimingGraph &graph, const std::string &filename,
                 const std::vector<CriticalPath> &setupPaths,
                 const std::vector<CriticalPath> &holdPaths);
void writeHeader(std::ostream &out);
void writeNodes(std::ostream &out, const TimingGraph &graph);
void writeEdges(
    std::ostream &out, const TimingGraph &graph,
    const std::unordered_map<EdgeKey, EdgeStyle, EdgeKeyHash> &edgeStyles);
void writeFooter(std::ostream &out);
void writeLegend(std::ostream &out);
} // namespace DotExporter

std::vector<CriticalPath> getWorstSetupPaths(const TimingGraph &graph,
                                             size_t count);
std::vector<CriticalPath> getWorstHoldPaths(const TimingGraph &graph,
                                            size_t count);

void buildEdgeStyles(
    std::unordered_map<EdgeKey, EdgeStyle, EdgeKeyHash> &edgeStyles,
    const std::vector<CriticalPath> &setupPaths,
    const std::vector<CriticalPath> &holdPaths);
