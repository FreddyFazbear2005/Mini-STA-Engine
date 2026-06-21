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
    ss >> keyword;

    if (!(ss >> keyword)) {
      continue;
    }

    if (keyword == "NODE") {
      parseNode(ss);
    } else if (keyword == "EDGE") {
      parseEdge(ss);
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
