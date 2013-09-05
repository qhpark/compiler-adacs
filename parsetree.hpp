#ifndef _PARSETREE_HPP_
#define _PARSETREE_HPP_

#include "token.hpp"
#include <string>


// Representation of a parse tree node.
class ParseTreeNode {

	// Output.
	friend std::ostream& operator<<( std::ostream &out, const ParseTreeNode &T);
	
	public:

		// Possible Types.
		//enum Type { EXPRESSION, STATEMENT, FUNCTION };

		// Constructor.
		ParseTreeNode(unsigned int nodeType);

		// Destructor.
		~ParseTreeNode();

		// Adding a created child node as the first child
		ParseTreeNode * addChildFront(ParseTreeNode * child);
	
		// Adding a created child node as the last child.
		ParseTreeNode * addChildBack(ParseTreeNode * child);

		// Check to see if this node is a token.
		bool hasToken() const;

		// Deal with terminals.
		void setToken(Token*);
		Token * getToken();

		// Access the tree structure.
		ParseTreeNode * getFirstChild();
		ParseTreeNode * getLastChild();
		ParseTreeNode * getParent();
		ParseTreeNode * getNextSibling();
		bool hasChild() const;


		void quickDump();
		void quickDump(std::string & parentPrefix);

		unsigned int getType() const;

		unsigned int getNumChild() const;

		void setName(char * name);

		char * getName();

	private:

		// One level down.
		ParseTreeNode * firstChild;
		ParseTreeNode * lastChild;

		// One Level Up.
		ParseTreeNode * parent;

		// Same Level.
		ParseTreeNode * sibling;

		//Type type;
		unsigned int type;
		Token * token;

		// Num child.
		unsigned int numChild;

		//
		char * name;
};

#endif
