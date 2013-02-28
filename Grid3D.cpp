#include <tulip/ImportModule.h>
#include <tulip/TulipPluginHeaders.h>
#include <math.h>
#include <stdexcept>

using namespace std;
using namespace tlp;

#define CHECK_PROP_PROVIDED(PROP, STOR) \
	do { \
		if(!dataSet->get(PROP, STOR)) \
			throw std::runtime_error(std::string("No \"") + PROP + "\" property provided."); \
	} while(0)

const string PLUGIN_NAME("Grid 3D");

namespace {
const char *paramHelp[] = {
	// Width
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Unsigned int")
		HTML_HELP_DEF("Default", "2")
		HTML_HELP_BODY()
		"This parameter defines the grid's width."
		HTML_HELP_CLOSE(),

	// Height
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Unsigned int")
		HTML_HELP_DEF("Default", "2")
		HTML_HELP_BODY()
		"This parameter defines the grid's height."
		HTML_HELP_CLOSE(),

	// Depth
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Unsigned int")
		HTML_HELP_DEF("Default", "2")
		HTML_HELP_BODY()
		"This parameter defines the grid's depth."
		HTML_HELP_CLOSE(),

	//Connectivity
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Integer")
		HTML_HELP_DEF("Values", "0;4;8")
		HTML_HELP_DEF("Default", "4")
		HTML_HELP_BODY()
		"This parameter defines the connectivity number of each node."
		HTML_HELP_CLOSE(),

	// Positionning
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Boolean")
		HTML_HELP_DEF("Default", "true")
		HTML_HELP_BODY()
		"This parameter indicates if the nodes should be positionned in space."
		HTML_HELP_CLOSE(),

	// Spacing
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Double")
		HTML_HELP_DEF("Default", "1.0")
		HTML_HELP_BODY()
		"This parameter defines the spacing between each node in the grid."
};
}

class Grid3D: public ImportModule {
public:
	PLUGININFORMATIONS(PLUGIN_NAME, "Cyrille Faucheux", "2011-06-23", "", "1.0", "Graph")

	Grid3D(PluginContext *context) :
		ImportModule(context)
	{
		addInParameter< unsigned int >     ( "Width",        paramHelp[0], "2");
		addInParameter< unsigned int >     ( "Height",       paramHelp[1], "2");
		addInParameter< unsigned int >     ( "Depth",        paramHelp[2], "2");
		addInParameter< StringCollection > ( "Connectivity", paramHelp[3], "0;4;8");
		addInParameter< bool >             ( "Positionning", paramHelp[4], "true");
		addInParameter< double >           ( "Spacing",      paramHelp[5], "1.0");
	}
	~Grid3D() {}

	bool importGraph()
	{
		int width = 10, height = 10, depth = 10, conn = 4;
		StringCollection connectivity;
		bool positionning = true;
		double spacing = 1.0;

		try {
			if(dataSet == NULL)
				throw std::runtime_error("No dataset provided.");

			CHECK_PROP_PROVIDED("Width",        width);
			CHECK_PROP_PROVIDED("Height",       height);
			CHECK_PROP_PROVIDED("Depth",        depth);
			CHECK_PROP_PROVIDED("Spacing",      spacing);
			CHECK_PROP_PROVIDED("Positionning", positionning);
			CHECK_PROP_PROVIDED("Connectivity", connectivity);

			if(width <= 0)
				throw std::runtime_error("Parameter \"Width\" must be a positive number");
			if(height <= 0)
				throw std::runtime_error("Parameter \"Height\" must be a positive number");
			if(depth <= 0)
				throw std::runtime_error("Parameter \"Depthh\" must be a positive number");

			if(spacing <= 0.0)
				throw std::runtime_error("Parameter \"Spacing\" must be a positive positive");

		} catch (std::runtime_error &ex) {
			if(this->pluginProgress) {
				this->pluginProgress->setError(ex.what());
			}
			return false;
		}

		graph->setAttribute("width",  width);
		graph->setAttribute("height", height);
		graph->setAttribute("depth",  depth);

		conn = (connectivity.getCurrentString().compare("8") == 0 ? 8 : (connectivity.getCurrentString().compare("4") == 0 ? 4 : 0));
		const bool c4 = (conn == 4);
		const bool c8 = (conn == 8);
		const int nbNodes = width * height * depth;

		vector<node> nodes; nodes.reserve(nbNodes);
		graph->addNodes(nbNodes, nodes);

		if(!positionning)
			graph->getProperty<LayoutProperty>("viewLayout")->setAllNodeValue(Coord(0, 0, 0));

		// compute nb edges
		unsigned int nbEdges = 0;
		if(c4 || c8) {
			nbEdges += (width - 1) * height; // X edges
			nbEdges += (height - 1) * width; // Y edges

			if(c8)
				nbEdges += 2 * (width - 1) * (height - 1); // XY edges

			nbEdges *= depth; // For each plane

			nbEdges += (depth - 1) * (width * height); // Z edges

			if(c8) {
				nbEdges += 2 * (width - 1) * height * (depth - 1); // XZ diagonal edges
				nbEdges += 2 * (height - 1) * width * (depth - 1); // YZ diagonal edges
				nbEdges += 4 * (width - 1) * (height - 1) * (depth - 1); // Cross-planes diagonal edges
			}
		}

		vector<pair<node, node> > ends; ends.reserve(nbEdges);
		vector<edge> edges; edges.reserve(nbEdges);

		int i = 0, j = 0, k = 0, progress = 0;
		const int lastW = width - 1, lastH = height - 1, lastD = depth - 1;
		const double s = (1.0 + spacing);
		vector<node>::const_iterator nodesIterator;
		LayoutProperty *layout = graph->getProperty<LayoutProperty> ("viewLayout");

#define REL_IT(it, dw, dh, dd) *((it + (dd) * width * height + (dh) * width + (dw)))
#define NOTLAST_W (i < lastW)
#define NOTLAST_H (j < lastH)
#define NOTLAST_D (k < lastD)
#define NOTFIRST_W (i > 0)
#define NOTFIRST_H (j > 0)
#define NOTFIRST_D (k > 0)

		if(pluginProgress)
			pluginProgress->setComment("Creating edges");

		for(nodesIterator = nodes.begin(); nodesIterator < nodes.end(); ++nodesIterator) {

			if(positionning) {
				layout->setNodeValue(*nodesIterator, Coord(i * s, j * s, k * s));
			}

			if(c4 || c8) {
				if(NOTLAST_W) {
					// X edges
					ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 1, 0, 0)));
				}
				if(NOTLAST_H) {
					// Y edges
					ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 0, 1, 0)));
				}
				if(NOTLAST_D) {
					// Z edges
					ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 0, 0, 1)));
				}

				if(c8) {
					if(NOTLAST_H) {
						// XY plane diagonal edges
						if(NOTFIRST_W) {
							ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, -1, 1, 0)));
						}
						if(NOTLAST_W) {
							ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 1, 1, 0)));
						}
					}

					if(NOTLAST_D) {
						// XZ plane diagonal edges
						if(NOTFIRST_W) {
							ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, -1, 0, 1)));
						}
						if(NOTLAST_W) {
							ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 1, 0, 1)));
						}

						// YZ plane diagonal edges
						if(NOTFIRST_H) {
							ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 0, -1, 1)));
						}
						if(NOTLAST_H) {
							ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 0, 1, 1)));
						}
					}

					// Cross plane diagonal edges
					if(NOTLAST_D) {
						if(NOTFIRST_W) {
							if(NOTFIRST_H) {
								ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, -1, -1, 1)));
							}
							if(NOTLAST_H) {
								ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, -1, 1, 1)));
							}
						}
						if(NOTLAST_W) {
							if(NOTFIRST_H) {
								ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 1, -1, 1)));
							}
							if(NOTLAST_H) {
								ends.push_back(pair<node, node>(*nodesIterator, REL_IT(nodesIterator, 1, 1, 1)));
							}
						}
					}
				}
			}

			++i;
			if(i == width) {
				i = 0;
				++j;
				if(j == height) {
					j = 0;
					++k;
				}
			}

			if(pluginProgress)
				pluginProgress->progress(++progress, nbNodes);
		}

		if(c4 || c8)
			graph->addEdges(ends, edges);

		return true;
	}
};

PLUGIN(Grid3D);
