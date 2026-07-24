#include "TimingGraph.hpp"
#include "Node.hpp"
#include <cstddef>
#include <stdexcept>

NodeID TimingGraph::addNode(const std::string &name, NodeType type) {
  NodeID currentID = this->nodes.size();
  this->nodes.emplace_back(currentID, name, type);
  return currentID;
}

Node &TimingGraph::getNode(NodeID id) { return this->nodes.at(id); }

const Node &TimingGraph::getNode(NodeID id) const { return this->nodes.at(id); }

const std::vector<Node> &TimingGraph::getNodes() const { return this->nodes; }

size_t TimingGraph::size() const { return this->nodes.size(); }

void TimingGraph::addEdge(NodeID src, NodeID dst, double maxDelay,
                          double minDelay) {
  // this->nodes.at(src).fanout.push_back(dst);
  // this->nodes.at(dst).fanin.push_back(src);
  const Node &sourceNode = nodes.at(src);
  for (EdgeID edgeID : sourceNode.outgoingEdges) {
    const Edge &edge = edges.at(edgeID);
    if (edge.destination == dst) {
      throw std::runtime_error("Duplicate EDGE : " + sourceNode.name + " -> " +
                               nodes.at(dst).name);
    }
  }
  EdgeID currentID = this->edges.size();
  this->nodes.at(src).outgoingEdges.push_back(currentID);
  this->nodes.at(dst).incomingEdges.push_back(currentID);
  this->edges.emplace_back(currentID, src, dst, maxDelay, minDelay);
}

Edge &TimingGraph::getEdge(EdgeID id) { return this->edges.at(id); }

const Edge &TimingGraph::getEdge(EdgeID id) const { return this->edges.at(id); }

const std::vector<Edge> &TimingGraph::getEdges() const { return this->edges; }
