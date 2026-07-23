#pragma once

#include "Node.hpp"

struct Edge {
  EdgeID id;

  NodeID source;
  NodeID destination;

  double maxDelay = 0.0;
  double minDelay = 0.0;

  Edge(EdgeID id, NodeID source, NodeID destination)
      : id(id), source(source), destination(destination) {}
};
