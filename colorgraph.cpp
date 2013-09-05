#include <iostream>
#include <algorithm>
#include <vector>
#include "colorgraph.hpp"

ColorGraphNode::ColorGraphNode(ColorGraphNodeIndex idx) :
  nodeIdx(idx), nodeColor(0) {}

ColorGraphNode::~ColorGraphNode() {}

void ColorGraphNode::connectTo(ColorGraphNodeIndex idx) {
	adj.push_back(idx);
}

unsigned int ColorGraphNode::degree() {
	return adj.size();
}

std::ostream& operator<<( std::ostream &out, const ColorGraphNode &n) {

	out << " CNode"<< &n << " idx=" << n.nodeIdx << " colour=" ;
	out << n.nodeColor << " ADJ={";
	for (std::vector<ColorGraphNodeIndex>::const_iterator i = n.adj.begin();
		 i != n.adj.end(); i++) {
		out << *i << " ";
	}
	out << "}";
	return out;
}

ColorGraph::ColorGraph() {}

ColorGraph::~ColorGraph() {}

ColorGraphNodeIndex ColorGraph::addNode() {
	ColorGraphNodeIndex idx = nodes.size();
	ColorGraphNode n(idx);
	nodes.push_back(n);
	return idx;
}

bool ColorGraph::connect(ColorGraphNodeIndex a, ColorGraphNodeIndex b) {
	unsigned int maxIdx = nodes.size() - 1;
	if (a > maxIdx || b >> maxIdx) {
		return false;
	} else {
		nodes[a].connectTo(b);
		nodes[b].connectTo(a);
	}
}

void ColorGraph::assignColor() {
	//sort(nodes.begin(), nodes.end(), ColorGraphNodeDegLess());
	std::vector<ColorGraphNode*> sortedNList;
	for (std::vector<ColorGraphNode>::iterator nodePtr = nodes.begin();
		 nodePtr != nodes.end(); nodePtr ++) {
		sortedNList.push_back(&(*nodePtr));
	}
	for (std::vector<ColorGraphNode*>::const_iterator nodePtr = sortedNList.begin();
		 nodePtr != sortedNList.end(); nodePtr ++) {

		bool foundColor = false;
		ColorGraphNodeColor color  = 0; 
		while (!foundColor) {
			color++;

			bool connectToColor = false;
			for (std::vector<ColorGraphNodeIndex>::const_iterator i = (*nodePtr)->adj.begin();
		 	     i != (*nodePtr)->adj.end(); i++) {
				if (nodes[*i].nodeColor == color) {
					connectToColor = true;
					break;
				}
			}

			foundColor = !connectToColor;
		}
		(*nodePtr)->nodeColor = color;
	}
}


std::ostream& operator<<( std::ostream &out, const ColorGraph &g) {
	out << "ColorGraph=" << &g << std::endl;
	for (std::vector<ColorGraphNode>::const_iterator i = g.nodes.begin();
		 i != g.nodes.end(); i++) {
		out << *i << std::endl;
	}
	return out;
}
