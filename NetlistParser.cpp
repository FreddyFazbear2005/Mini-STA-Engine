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
    } else if (keyword == "SETUP") {
      parseSetup(ss);
    } else if (keyword == "HOLD") {
      parseHold(ss);
    } else if (keyword == "CLOCK_PERIOD") {
      parseClockPeriod(ss);
    } else {
      throw std::runtime_error("Unknown keyword: " + keyword);
    }
  }
  file.close();
  validate();
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
    throw std::runtime_error("Malformed DELAY statement");
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

  if (node.hasDelay == true) {
    throw std::runtime_error("DUPLICATE DELAY FOR NODE " + node.name);
  }

  node.maxCellDelay = maxCellDelay;
  node.minCellDelay = minCellDelay;
  node.hasDelay = true;
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
  if (node.hasClockToQ == true) {
    throw std::runtime_error("DUPLICATE CLOCK_TO_Q FOR NODE " + node.name);
  }
  node.clockToQ = clockToQ;
  node.hasClockToQ = true;
}

void NetListParser::parseSetup(std::stringstream &ss) {
  std::string nodeName;
  double setupTime;

  if (!(ss >> nodeName >> setupTime)) {
    throw std::runtime_error("Malformed SETUP statement");
  }

  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in SETUP statement: " + extra);
  }

  if (nodeMap.find(nodeName) == nodeMap.end()) {
    throw std::runtime_error("SETUP references undefined node: " + nodeName);
  }

  if (setupTime < 0) {
    throw std::runtime_error("SETUP cannot be NEGATIVE");
  }

  NodeID nodeID = nodeMap.at(nodeName);
  Node &node = graph.getNode(nodeID);
  if (node.type != NodeType::flipFlopD) {
    throw std::runtime_error("SETUP can only be applied to FF_D nodes");
  }
  if (node.hasSetupTime == true) {
    throw std::runtime_error("DUPLICATE SETUP TIME FOR NODE " + node.name);
  }
  node.setupTime = setupTime;
  node.hasSetupTime = true;
}

void NetListParser::parseHold(std::stringstream &ss) {
  std::string nodeName;
  double holdTime;

  if (!(ss >> nodeName >> holdTime)) {
    throw std::runtime_error("Malformed HOLD statement");
  }

  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in HOLD statement: " + extra);
  }

  if (nodeMap.find(nodeName) == nodeMap.end()) {
    throw std::runtime_error("HOLD references undefined node: " + nodeName);
  }

  if (holdTime < 0) {
    throw std::runtime_error("HOLD cannot be NEGATIVE");
  }

  NodeID nodeID = nodeMap.at(nodeName);
  Node &node = graph.getNode(nodeID);
  if (node.type != NodeType::flipFlopD) {
    throw std::runtime_error("HOLD can only be applied to FF_D nodes");
  }
  if (node.hasHoldTime == true) {
    throw std::runtime_error("DUPLICATE HOLD TIME FOR NODE " + node.name);
  }
  node.holdTime = holdTime;
  node.hasHoldTime = true;
}

void NetListParser::parseClockPeriod(std::stringstream &ss) {
  if (clockPeriod != -1) {
    throw std::runtime_error("CLOCK_PERIOD specified more than once");
  }
  if (!(ss >> clockPeriod)) {
    throw std::runtime_error("Malformed CLOCK_PERIOD statement");
  }
  std::string extra;
  if (ss >> extra) {
    throw std::runtime_error("Unexpected token in CLOCK_PERIOD statement: " +
                             extra);
  }
  if (clockPeriod <= 0) {
    throw std::runtime_error("CLOCK_PERIOD must be greater than zero");
  }
}

double NetListParser::getClockPeriod() const { return clockPeriod; }

void NetListParser::validate() const {
  if (clockPeriod < 0) {
    throw std::runtime_error("CLOCK_PERIOD not specified");
  }
  for (const Node &currentNode : graph.getNodes()) {
    if (currentNode.type == NodeType::primaryInput) {
      if (currentNode.fanin.empty() == false) {
        throw std::runtime_error("PRIMARY INPUT " + currentNode.name +
                                 " SHOULD NOT HAVE FANIN");
      }
      if (currentNode.fanout.empty() == true) {
        throw std::runtime_error("PRIMARY INPUT " + currentNode.name +
                                 " HAS NO FANOUT");
      }
    } else if (currentNode.type == NodeType::primaryOutput) {
      if (currentNode.fanin.empty() == true) {
        throw std::runtime_error("PRIMARY OUTPUT " + currentNode.name +
                                 " HAS NO FANIN");
      }
      if (currentNode.fanout.empty() == false) {
        throw std::runtime_error("PRIMARY OUTPUT " + currentNode.name +
                                 " SHOULD NOT HAVE FANOUT");
      }
    } else if (currentNode.type == NodeType::gate) {
      if (currentNode.fanin.empty() == true) {
        throw std::runtime_error("GATE " + currentNode.name + " HAS NO FANIN");
      }
      if (currentNode.fanout.empty() == true) {
        throw std::runtime_error("GATE " + currentNode.name + " HAS NO FANOUT");
      }
      if (currentNode.hasDelay == false) {
        throw std::runtime_error("GATE " + currentNode.name + " HAS NO DELAY");
      }
    } else if (currentNode.type == NodeType::flipFlopQ) {
      if (currentNode.fanin.empty() == false) {
        throw std::runtime_error("FLIP FLOP Q " + currentNode.name +
                                 " CANNOT HAVE FANIN");
      }
      if (currentNode.hasClockToQ == false) {
        throw std::runtime_error("FLIP FLOP Q " + currentNode.name +
                                 " HAS NO CLOCK_TO_Q");
      }
      if (currentNode.fanout.empty() == true) {
        throw std::runtime_error("FLIP FLOP Q " + currentNode.name +
                                 " HAS NO FANOUT");
      }
    } else if (currentNode.type == NodeType::flipFlopD) {
      if (currentNode.fanin.empty() == true) {
        throw std::runtime_error("FLIP FLOP D " + currentNode.name +
                                 " HAS NO FANIN");
      }
      if (currentNode.fanout.empty() == false) {
        throw std::runtime_error("FLIP FLOP D " + currentNode.name +
                                 " CANNOT HAVE FANOUT");
      }
      if (currentNode.hasSetupTime == false) {
        throw std::runtime_error("FLIP FLOP D " + currentNode.name +
                                 " HAS NO SETUP TIME");
      }
      if (currentNode.hasHoldTime == false) {
        throw std::runtime_error("FLIP FLOP D " + currentNode.name +
                                 " HAS NO HOLD TIME");
      }
    }
  }
}
