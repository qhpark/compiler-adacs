#ifndef _COLOR_GRAPH_
#define _COLOR_GRAPH_

#include <vector>
#include <functional>

typedef unsigned int ColorGraphNodeIndex;
typedef unsigned int ColorGraphNodeColor;


class ColorGraph;
class ColorGraphNode {
	friend std::ostream& operator<<( std::ostream &out, const ColorGraphNode &c);
	friend class ColorGraph;
	public:
		ColorGraphNode(ColorGraphNodeIndex);
		~ColorGraphNode();

		// Connect to another node.
		void connectTo(ColorGraphNodeIndex);

		unsigned int degree();

	private:

		// Index.
		ColorGraphNodeIndex nodeIdx;
		ColorGraphNodeColor nodeColor;

		// Adjacent List.
		std::vector<ColorGraphNodeIndex> adj;
};

struct ColorGraphNodeDegLess : 
  public std::binary_function<ColorGraphNode, ColorGraphNode, bool> {
	bool operator()(ColorGraphNode a, ColorGraphNode b) {
		return a.degree() < b.degree();
	}
};

class ColorGraph {
	friend std::ostream& operator<<( std::ostream &out, const ColorGraph &c);
	public:
		ColorGraph();
		~ColorGraph();
		void assignColor();

		// Add
		ColorGraphNodeIndex addNode();

		// Connect
		bool connect(ColorGraphNodeIndex, ColorGraphNodeIndex);
	private:
		std::vector<ColorGraphNode> nodes;
};

#endif
