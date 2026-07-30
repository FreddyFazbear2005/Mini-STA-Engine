#pragma once

#include "TimingGraph.hpp"
#include <cstddef>
#include <iosfwd>
#include <vector>

namespace DotExporter {
void exportGraph(const TimingGraph &graph, const std::string &filename);
void writeHeader(std::ostream &out);
void writeNodes(std::ostream &out, const TimingGraph &graph);
void writeEdges(std::ostream &out, const TimingGraph &graph);
void writeFooter(std::ostream &out);
} // namespace DotExporter

struct CriticalPath {
  std::vector<NodeID> nodes;
  double slack;
  double delay;
  NodeID endNodeID;
  NodeID startNodeID;
};

std::vector<CriticalPath> getWorstSetupPaths(const TimingGraph &graph,
                                             size_t count);
std::vector<CriticalPath> getWorstHoldPaths(const TimingGraph &graph,
                                            size_t count);
