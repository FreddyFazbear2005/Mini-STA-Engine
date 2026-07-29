#include "DotExporter.hpp"
#include "Node.hpp"
#include <fstream>
#include <stdexcept>

void DotExporter::exportGraph(const TimingGraph &graph,
                              const std::string &filename) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("Cannot open " + filename);
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
