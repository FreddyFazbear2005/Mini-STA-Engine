#include "Cell.hpp"
#include <stdexcept>

void CellLibrary::addCell(const std::string &name, double maxDelay,
                          double minDelay) {
  if (cells.find(name) != cells.end()) {
    throw std::runtime_error("ERROR: Duplicate definition of " + name);
  }
  cells.emplace(name, Cell(name, maxDelay, minDelay));
}

const Cell &CellLibrary::getCell(const std::string &name) const {
  auto it = cells.find(name);
  if (it == cells.end()) {
    throw std::runtime_error("ERROR: Unknown cell: " + name);
  }
  return it->second;
}
