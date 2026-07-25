#pragma once

#include <string>
#include <unordered_map>

struct Cell {
  std::string name;
  double maxDelay;
  double minDelay;

  Cell(const std::string &cellName, double maxDelay, double minDelay)
      : name(cellName), maxDelay(maxDelay), minDelay(minDelay) {}
};

class CellLibrary {
public:
  void addCell(const std::string &name, double maxDelay, double minDelay);

  const Cell &getCell(const std::string &name) const;

private:
  std::unordered_map<std::string, Cell> cells;
};
