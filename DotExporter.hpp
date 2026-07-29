#pragma once

#include "TimingGraph.hpp"
#include <iosfwd>

namespace DotExporter {
void exportGraph(const TimingGraph &graph, const std::string &filename);
void writeHeader(std::ostream &out);
void writeNodes(std::ostream &out, const TimingGraph &graph);
void writeEdges(std::ostream &out, const TimingGraph &graph);
void writeFooter(std::ostream &out);
} // namespace DotExporter
