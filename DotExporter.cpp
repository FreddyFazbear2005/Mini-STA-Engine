#include "DotExporter.hpp"
#include <fstream>
#include <stdexcept>

void DotExporter::exportGraph(const TimingGraph &graph,
                              const std::string &filename) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("Cannot open " + filename);
  }
  file << "digraph TimingGraph {\n";
  file << "    rankdir=LR;\n\n";

  for (const auto &node : graph.getNodes()) {
    file << "    " << node.name << ";\n";
  }
  file << "\n";

  for (const auto &edge : graph.getEdges()) {
    const Node &src = graph.getNode(edge.source);
    const Node &dst = graph.getNode(edge.destination);

    file << "    " << src.name << " -> " << dst.name << ";\n";
  }
  file << "}\n";
  file.close();
}
