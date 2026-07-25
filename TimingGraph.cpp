#include "TimingGraph.hpp"
#include <cstddef>
#include <stdexcept>

NodeID TimingGraph::addNode(const std::string &name, NodeType type,
                            const std::string &cellType, double maxCellDelay,
                            double minCellDelay) {
  NodeID currentID = nodes.size();
  nodes.emplace_back(currentID, name, type, cellType, maxCellDelay,
                     minCellDelay);
  return currentID;
}

Node &TimingGraph::getNode(NodeID id) { return nodes.at(id); }

const Node &TimingGraph::getNode(NodeID id) const { return nodes.at(id); }

const std::vector<Node> &TimingGraph::getNodes() const { return nodes; }

size_t TimingGraph::size() const { return nodes.size(); }

void TimingGraph::addEdge(NodeID src, NodeID dst, double maxDelay,
                          double minDelay) {
  const Node &sourceNode = nodes.at(src);
  for (EdgeID edgeID : sourceNode.outgoingEdges) {
    const Edge &edge = edges.at(edgeID);
    if (edge.destination == dst) {
      throw std::runtime_error("Duplicate EDGE : " + sourceNode.name + " -> " +
                               nodes.at(dst).name);
    }
  }
  EdgeID currentID = edges.size();
  edges.emplace_back(currentID, src, dst, maxDelay, minDelay);
  nodes.at(src).outgoingEdges.push_back(currentID);
  nodes.at(dst).incomingEdges.push_back(currentID);
}

Edge &TimingGraph::getEdge(EdgeID id) { return edges.at(id); }

const Edge &TimingGraph::getEdge(EdgeID id) const { return edges.at(id); }

const std::vector<Edge> &TimingGraph::getEdges() const { return edges; }
