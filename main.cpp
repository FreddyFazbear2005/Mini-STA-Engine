#include "Cell.hpp"
#include "LibraryParser.hpp"
#include "NetlistParser.hpp"
#include "STAEngine.hpp"
#include "TimingGraph.hpp"

int main() {
  TimingGraph graph;
  LibraryParser libraryParser;
  CellLibrary cellLibrary;

  libraryParser.parseLibraryFile("lib.txt", cellLibrary);
  NetListParser parser(graph);
  parser.parse("STA.txt", cellLibrary);

  STAEngine sta(graph);

  sta.run(parser.getClockPeriod(), parser.getClockUncertainty());

  sta.displayTimingReport();
  sta.displaySetupCriticalPaths(5);
  sta.displayHoldCriticalPaths(5);
}
