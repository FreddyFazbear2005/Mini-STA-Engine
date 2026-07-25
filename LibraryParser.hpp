#pragma once

#include "Cell.hpp"

class LibraryParser {
public:
  void parseLibraryFile(const std::string &filename, CellLibrary &cellLibrary);
};
