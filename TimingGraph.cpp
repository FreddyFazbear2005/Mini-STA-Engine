#include "TimingGraph.hpp"
#include <cstddef>

NodeID TimingGraph::addNode(const std::string &name, NodeType type) {
  NodeID currentID = this->nodes.size();
  this->nodes.emplace_back(currentID, name, type);
  return currentID;
}

Node &TimingGraph::getNode(NodeID id) { return this->nodes.at(id); }

const Node &TimingGraph::getNode(NodeID id) const { return this->nodes.at(id); }

const std::vector<Node> &TimingGraph::getNodes() const { return this->nodes; }

size_t TimingGraph::size() const { return this->nodes.size(); }

void TimingGraph::addEdge(NodeID src, NodeID dst) {
  this->nodes.at(src).fanout.push_back(dst);
  this->nodes.at(dst).fanin.push_back(src);
}
