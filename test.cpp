#include <iostream>
#include <tulip/Graph.h>

#include <tulip/TlpTools.h>
#include <tulip/TulipPlugin.h>

using namespace std;
using namespace tlp;

int main(int argc, char* argv[]) {
  if(argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " <path to tulip install>" << std::endl;
    exit(0);
  }

  tlp::initTulipLib(argv[1]);
  tlp::loadPlugins();

  DataSet data;
  data.set<int>("Width", 20);
  data.set<int>("Height", 20);
  data.set<int>("Depth", 20);
  data.set<StringCollection>("Connectivity", StringCollection("4"));
  data.set<bool>("Positionning", true);
  data.set<double>("Spacing", 1.0);

  Graph *graph = tlp::importGraph("Grid 3D", data);

  tlp::saveGraph(graph, "graph.tlp");

  //delete the graph
  delete graph;
  return EXIT_SUCCESS;
}