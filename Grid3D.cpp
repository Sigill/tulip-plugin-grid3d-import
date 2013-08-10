#include <tulip/ImportModule.h>
#include <tulip/TulipPluginHeaders.h>
#include <tulip/StringCollection.h>
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

#define NTYPES "Circular;Square"

namespace {
const char * NeighorhoodTypes = NTYPES;
const char *paramHelp[] = {
	// 0 Width
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Unsigned int")
		HTML_HELP_DEF("Default", "2")
		HTML_HELP_BODY()
		"The grid's width."
		HTML_HELP_CLOSE(),

	// 1 Height
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Unsigned int")
		HTML_HELP_DEF("Default", "2")
		HTML_HELP_BODY()
		"The grid's height."
		HTML_HELP_CLOSE(),

	// 2 Depth
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Unsigned int")
		HTML_HELP_DEF("Default", "2")
		HTML_HELP_BODY()
		"The grid's depth."
		HTML_HELP_CLOSE(),

	// 3 Neighborhood radius
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Double")
		HTML_HELP_DEF("Default", "0")
		HTML_HELP_BODY()
		"The radius of the generated neighborhood (this is not affected by the Spacing parameter)."
		HTML_HELP_CLOSE(),

	// 4 Neighborhood type
	HTML_HELP_OPEN()
		HTML_HELP_DEF("type", "String")
		HTML_HELP_DEF("Values", NTYPES)
		HTML_HELP_DEF("Default", "Circular")
		HTML_HELP_BODY()
		"The type of neighborhood to build."
		HTML_HELP_CLOSE(),

	// 5 Positionning
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Boolean")
		HTML_HELP_DEF("Default", "true")
		HTML_HELP_BODY()
		"Indicates if the nodes should be positionned in space."
		HTML_HELP_CLOSE(),

	// 6 Spacing
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "Double")
		HTML_HELP_DEF("Default", "1.0")
		HTML_HELP_BODY()
		"The spacing between each node in the grid."
};
}

// This class is to be replaced by tlp::Vec3i as soon as
// https://sourceforge.net/p/auber/bugs/728/ is corrected
// //////////////////////////////////////////////////////
class Vec3int {
private:
	int _x, _y, _z;

public:
	Vec3int():
		_x(0), _y(0), _z(0) {}

	Vec3int(int __x, int __y, int __z):
		_x(__x), _y(__y), _z(__z) {}

	Vec3int(const Vec3int &o):
		_x(o.x()), _y(o.y()), _z(o.z()) {}

	int x() const { return _x; }
	int y() const { return _y; }
	int z() const { return _z; }

	Vec3int& operator+=(const Vec3int& o)
	{
		_x += o.x();
		_y += o.y();
		_z += o.z();

		return *this;
	}

	Vec3int& operator-=(const Vec3int& o)
	{
		_x -= o.x();
		_y -= o.y();
		_z -= o.z();

		return *this;
	}

	double norm() const
	{
		return sqrt(_x*_x + _y*_y + _z*_z);
	}

	double dist(const Vec3int& o) const;
};

Vec3int operator+(const Vec3int &a, const Vec3int &b)
{
	return Vec3int(a) += b;
}

Vec3int operator-(const Vec3int &a, const Vec3int &b)
{
	return Vec3int(a) -= b;
}

double Vec3int::dist(const Vec3int& o) const
{
	Vec3int v = Vec3int(*this) - o;
	return v.norm();
}

/*
ostream& operator<<(ostream& out, const Vec3int& vec)
{
	out << "(" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")";
	return out;
}
*/
// //////////////////////////////////////////////////////

void buildManhattanNeighborhood(std::vector< Vec3int > *offsets, float radius, bool is2D)
{
	int r = (int)radius;

	for(int k = -r; k <= r; ++k) {
		if(is2D && k != 0)
			continue;

		for(int j = -r; j <= r; ++j) {
			for(int i = -r; i <= r; ++i) {
				if(!(i == 0 && j == 0 && k == 0))
					offsets->push_back(Vec3int(i, j, k));
			}
		}
	}
}

void buildEuclidianNeighborhood(std::vector< Vec3int > *offsets, float radius, bool is2D)
{
	int r = (int)radius;

	for(int k = -r; k <= r; ++k) {
		if(is2D && k != 0)
			continue;

		for(int j = -r; j <= r; ++j) {
			for(int i = -r; i <= r; ++i) {
				if(!(i == 0 && j == 0 && k == 0)) {
					Vec3int offset(i, j, k);

					// std::cout << "Norm of " << offset << " is: " << offset.norm() << std::endl;

					if(offset.norm() <= radius)
						offsets->push_back(offset);
				}
			}
		}
	}
}

class Grid3D: public ImportModule {
public:
	PLUGININFORMATIONS(PLUGIN_NAME, "Cyrille Faucheux", "2011-06-23", "", "1.1", "Graph")

	Grid3D(PluginContext *context) :
		ImportModule(context)
	{
		addInParameter< unsigned int >     ("Width",                 paramHelp[0], "2");
		addInParameter< unsigned int >     ("Height",                paramHelp[1], "2");
		addInParameter< unsigned int >     ("Depth",                 paramHelp[2], "2");
		addInParameter< double >           ("Neighborhood radius",   paramHelp[3], "0");
		addInParameter< StringCollection > ("Neighborhood type",     paramHelp[4], NeighorhoodTypes, false);
		addInParameter< bool >             ("Positionning",          paramHelp[5], "true");
		addInParameter< double >           ("Spacing",               paramHelp[6], "1.0");
	}
	~Grid3D() {}

	bool importGraph()
	{
		int width = 10, height = 10, depth = 10;
		double neighborhood_radius;
		StringCollection neighborhood_type;
		bool positionning = true;
		double spacing = 1.0;
		std::vector<Vec3int> offsets;

		try {
			if(dataSet == NULL)
				throw std::runtime_error("No dataset provided.");

			CHECK_PROP_PROVIDED("Width",               width);
			CHECK_PROP_PROVIDED("Height",              height);
			CHECK_PROP_PROVIDED("Depth",               depth);
			CHECK_PROP_PROVIDED("Spacing",             spacing);
			CHECK_PROP_PROVIDED("Positionning",        positionning);
			CHECK_PROP_PROVIDED("Neighborhood radius", neighborhood_radius);

			if(width <= 0)
				throw std::runtime_error("Parameter \"Width\" must be positive");
			if(height <= 0)
				throw std::runtime_error("Parameter \"Height\" must be positive");
			if(depth <= 0)
				throw std::runtime_error("Parameter \"Depth\" must be positive");

			if(spacing <= 0.0)
				throw std::runtime_error("Parameter \"Spacing\" must be positive");

			if(neighborhood_radius >= 0) {
				if(neighborhood_radius > 0) {
					CHECK_PROP_PROVIDED("Neighborhood type", neighborhood_type);

					if(neighborhood_type.getCurrentString().compare("Circular") == 0) {
						buildEuclidianNeighborhood(&offsets, neighborhood_radius, depth == 1);
					} else if(neighborhood_type.getCurrentString().compare("Square") == 0) {
						buildManhattanNeighborhood(&offsets, neighborhood_radius, depth == 1);
					} else {
						throw std::runtime_error("Unknown neighborhood type.");
					}
				}
			} else {
				throw std::runtime_error("Parameter \"Neighborhood radius\" must be positive or null");
			}

		} catch (std::runtime_error &ex) {
			if(this->pluginProgress)
				this->pluginProgress->setError(ex.what());

			return false;
		}

		graph->setAttribute("width",  width);
		graph->setAttribute("height", height);
		graph->setAttribute("depth",  depth);

		const int nbNodes = width * height * depth;

		vector<node> nodes; nodes.reserve(nbNodes);
		graph->addNodes(nbNodes, nodes);

		if(!positionning)
			graph->getProperty<LayoutProperty>("viewLayout")->setAllNodeValue(Coord(0, 0, 0));

		SizeProperty *size = graph->getProperty< SizeProperty > ("viewSize");
		size->setAllNodeValue(Size(spacing / 2.0, spacing / 2.0, spacing / 2.0));

		int i = 0, j = 0, k = 0, progress = 0;
		vector<node>::const_iterator nodesIterator;
		LayoutProperty *layout = graph->getProperty< LayoutProperty > ("viewLayout");

#define REL_IT(it, dw, dh, dd) *((it + (dd) * width * height + (dh) * width + (dw)))

		if(pluginProgress)
			pluginProgress->setComment("Positionning nodes and creating edges");

		for(nodesIterator = nodes.begin(); nodesIterator < nodes.end(); ++nodesIterator) {

			if(positionning) {
				layout->setNodeValue(*nodesIterator, Coord(i * spacing, j * spacing, k * spacing));
			}

			if(!offsets.empty()) {
				Vec3int currentPosition(i, j, k);

				for(std::vector< Vec3int >::const_iterator offsetIterator = offsets.begin(); offsetIterator < offsets.end(); ++offsetIterator) {
					Vec3int offset(*offsetIterator);
					Vec3int otherPosition = currentPosition + offset;

					if((otherPosition.x() >= 0) && (otherPosition.x() < width) &&
					   (otherPosition.y() >= 0) && (otherPosition.y() < height) &&
					   (otherPosition.z() >= 0) && (otherPosition.z() < depth))
					{
						node v = REL_IT(nodesIterator, offset.x(), offset.y(), offset.z());
						if(!graph->existEdge(*nodesIterator, v, false).isValid())
							graph->addEdge(*nodesIterator, v);
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

		return true;
	}
};

PLUGIN(Grid3D);
