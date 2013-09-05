#include <iostream>
#include <string>
#include "parsetree.hpp"
#include "debug.hpp"

ParseTreeNode::ParseTreeNode(unsigned int t) :
  firstChild(0), lastChild(0), parent(0), sibling(0), numChild(0), type(t), token(0), name(0) {
}

ParseTreeNode::~ParseTreeNode() { }

unsigned int ParseTreeNode::getType() const { return type; }
unsigned int ParseTreeNode::getNumChild() const { return numChild; }

// Some not too interesting OO designed code.
ParseTreeNode * ParseTreeNode::getFirstChild() { return firstChild; }
ParseTreeNode * ParseTreeNode::getLastChild() { return lastChild; }
ParseTreeNode * ParseTreeNode::getParent() { return parent; }
ParseTreeNode * ParseTreeNode::getNextSibling() { return sibling; }

// Name....It would be nice to use some sort of index.
void ParseTreeNode::setName(char * n) { name = n; }
char * ParseTreeNode::getName() { return name; }

// Access some informationi about the node if it is a non-terminal.
bool ParseTreeNode::hasToken() const { return token; }

void ParseTreeNode::setToken(Token * t) {
	ASSERT(numChild == 0, "A parse tree node with children cannot be a terminal");
	token = t;
}

Token * ParseTreeNode::getToken() {
	return token;
}


// Add to the end of the siblings link.
ParseTreeNode * ParseTreeNode::addChildBack(ParseTreeNode * child) {

	++numChild;

	child->parent = this;
	if (firstChild == 0) {
		// This is the only child.
		firstChild = child;
		lastChild = child;
	} else {
		// Introduce it to its siblings.
		lastChild->sibling = child;
		lastChild = child;
	}

	return child;
}

// Add to the front of the siblings link.
ParseTreeNode * ParseTreeNode::addChildFront(ParseTreeNode * child) {

	++numChild;

	child->parent = this;
	if (firstChild == 0) {
		// This is the only child.
		firstChild = child;
		lastChild = child;
	} else {
		// Introduce it to its siblings.
		child->sibling = firstChild;
		firstChild = child;
	}

	return child;
}


bool ParseTreeNode::hasChild() const {
	return numChild > 0;
}

void ParseTreeNode::quickDump() {
	std::string pString("");
	quickDump(pString);
}

void ParseTreeNode::quickDump(std::string & parentStr) {
	std::string childStr = parentStr;
	if (parent != 0) {
		if (parent->lastChild != this) {
			childStr += "   |";
		} else {
			childStr += "    ";
		}
	}
	if (parent != 0) {
		std::cout << parentStr << "   |" << std::endl;
		std::cout << parentStr << "   |--";
	} else {
		std::cout << parentStr << "  ";
	}

	std::cout << *this << std::endl;

	if (firstChild != 0)
		firstChild->quickDump(childStr);
	if (sibling != 0)
		sibling->quickDump(parentStr);
}

std::ostream& operator<<( std::ostream &out, const ParseTreeNode &n) {

	if (n.hasToken()) {
		out << *(n.token) << " ";
	} else {
		out << "[";
		if (n.name) {
			out << n.name << " ";
		}
		out << "type=" << n.type << " ]" << " ";
	}

	return out;
}
