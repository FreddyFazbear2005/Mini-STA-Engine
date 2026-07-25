#include "LibraryParser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

void LibraryParser::parseLibraryFile(const std::string &filename,
                                     CellLibrary &cellLibrary) {
  std::ifstream file(filename);

  if (file.is_open() == false) {
    throw std::runtime_error("Error opening the file: " + filename);
  }

  std::string line;
  int lineNumber = 0;
  while (std::getline(file, line)) {
    lineNumber++;
    if (line.empty() == true) {
      continue;
    }

    if (line.find_first_not_of(" \t\r") == std::string::npos) {
      continue;
    }

    if (line[0] == '#') { // skips comments
      continue;
    }

    std::stringstream ss(line);

    std::string cellName;
    double minDelay;
    double maxDelay;

    if (!(ss >> cellName >> maxDelay >> minDelay)) {
      throw std::runtime_error("ERROR: Invalid Library entry at " +
                               std::to_string(lineNumber));
    }

    std::string extra;
    if (ss >> extra) {
      throw std::runtime_error(
          "ERROR: Unexpected token in Library entry : " + extra +
          " at line number" + std::to_string(lineNumber));
    }
    if (minDelay < 0 || maxDelay < 0) {
      throw std::runtime_error("ERROR: Negative Delay at line number " +
                               std::to_string(lineNumber));
    }
    if (maxDelay < minDelay) {
      throw std::runtime_error(
          "ERROR: maxDelay is less than minDelay at line number " +
          std::to_string(lineNumber));
    }
    cellLibrary.addCell(cellName, maxDelay, minDelay);
  }
  file.close();
}
