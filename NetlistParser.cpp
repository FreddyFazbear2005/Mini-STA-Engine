#include "NetlistParser.hpp"
#include "Node.hpp"
#include "TimingGraph.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

NetListParser::NetListParser(TimingGraph &graph) : graph(graph) {};

void NetListParser::parse(const std::string &filename) {
  std::ifstream file(filename);

  if (file.is_open() == false) {
    throw std::runtime_error("Error opening the file: " + filename);
  }
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() == true) {
      continue;
    }

    if (line[0] == '#') { // skips comments
      continue;
    }

    std::stringstream ss(line);
    std::string keyword;

    if (!(ss >> keyword)) {
      continue;
    }

    if (keyword == "NODE") {
      parseNode(ss);
    } else if (keyword == "EDGE") {
      parseEdge(ss);
    } else if (keyword == "DELAY") {
      parseDelay(ss);
    } else if (keyword == "CLOCK_TO_Q") {
      parseClockToQ(ss);
    }
  }
}

void NetListParser::parseNode(std::stringstream &ss) {
  std::string nodeName;
  std::string nodeType;

  if (!(ss >> nodeName >> nodeType)) {
    throw std::runtime_error("Malformed NODE statement");
  }

  NodeType type;
  if (nodeType == "GATE") {
    type = NodeType::gate;
  } else if (nodeType == "FF_Q") {
    type = NodeType::flipFlopQ;
  } else if (nodeType == "FF_D") {
    type = NodeType::flipFlopD;
  } else if (nodeType == "PI") {
    type = NodeType::primaryInput;
  } else if (nodeType == "PO") {
    type = NodeType::primaryOutput;
  } else {
    throw std::runtime_error("Invalid node type: " + nodeType);
  }

  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in NODE statement: " + extra);
  }

  if (nodeMap.find(nodeName) != nodeMap.end()) {
    throw std::runtime_error("Duplicate Node name: " + nodeName);
  }

  NodeID id = graph.addNode(nodeName, type);
  nodeMap[nodeName] = id;
}

void NetListParser::parseEdge(std::stringstream &ss) {
  std::string sourceName;
  std::string destinationName;

  if (!(ss >> sourceName >> destinationName)) {
    throw std::runtime_error("Malformed EDGE statement");
  }

  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in EDGE statement: " + extra);
  }

  if (nodeMap.find(sourceName) == nodeMap.end()) {
    throw std::runtime_error("Source Node " + sourceName + " does not exist");
  }

  if (nodeMap.find(destinationName) == nodeMap.end()) {
    throw std::runtime_error("Destination Node " + destinationName +
                             " does not exist");
  }

  NodeID sourceID = nodeMap.at(sourceName);
  NodeID destinationID = nodeMap.at(destinationName);

  graph.addEdge(sourceID, destinationID);
}

void NetListParser::parseDelay(std::stringstream &ss) {
  std::string nodeName;
  double maxCellDelay;
  double minCellDelay;

  if (!(ss >> nodeName >> maxCellDelay >> minCellDelay)) {
    throw std::runtime_error("malformed DELAY statement");
  }

  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in DELAY statement: " + extra);
  }

  if (nodeMap.find(nodeName) == nodeMap.end()) {
    throw std::runtime_error("DELAY references undefined node: " + nodeName);
  }

  if (maxCellDelay < minCellDelay) {
    throw std::runtime_error("Max cell delay is lesser than min cell delay");
  }

  if (maxCellDelay < 0 || minCellDelay < 0) {
    throw std::runtime_error("Cell delays cannot be NEGATIVE");
  }

  NodeID nodeID = nodeMap.at(nodeName);
  Node &node = graph.getNode(nodeID);

  if (node.type != NodeType::gate) {
    throw std::runtime_error("DELAY can only be applied to GATE nodes");
  }

  node.maxCellDelay = maxCellDelay;
  node.minCellDelay = minCellDelay;
}

void NetListParser::parseClockToQ(std::stringstream &ss) {
  std::string nodeName;
  double clockToQ;
  if (!(ss >> nodeName >> clockToQ)) {
    throw std::runtime_error("Malformed CLOCK_TO_Q statement");
  }

  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in CLOCK_TO_Q statement: " +
                             extra);
  }

  if (nodeMap.find(nodeName) == nodeMap.end()) {
    throw std::runtime_error("CLOCK_TO_Q references undefined node: " +
                             nodeName);
  }

  if (clockToQ < 0) {
    throw std::runtime_error("Clock to Q cannot be NEGATIVE");
  }

  NodeID nodeID = nodeMap.at(nodeName);
  Node &node = graph.getNode(nodeID);
  if (node.type != NodeType::flipFlopQ) {
    throw std::runtime_error("CLOCK_TO_Q can only be applied to FF_Q nodes");
  }
  node.clockToQ = clockToQ;
}
