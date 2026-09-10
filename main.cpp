#include "Cell.hpp"
#include "DotExporter.hpp"
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
  int numberOfPaths = 5;
  sta.displaySetupCriticalPaths(numberOfPaths);
  sta.displayHoldCriticalPaths(numberOfPaths);
  std::vector<CriticalPath> setupPaths =
      getWorstSetupPaths(graph, numberOfPaths);
  std::vector<CriticalPath> holdPaths = getWorstHoldPaths(graph, numberOfPaths);

  DotExporter::exportGraph(graph, "timing_graph.dot", setupPaths, holdPaths);
}
