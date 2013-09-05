#include "extree.hpp"
#include <iostream>

using namespace std;

ostream& operator<<(ostream &out, const ExTreeNode &T) {
    out<<"["<<T.nodename<<"]"<<endl;
}

// ExtreeNode Definitions
ExTreeNode::ExTreeNode(string name):nodename(name) { } 

ExTreeNode::~ExTreeNode() { } 

void ExTreeNode::quickDump() const { 
	quickDump(0);
} 
 
void ExTreeNode::quickDump(unsigned int depth) const {

} 


void PackageBodyNode::quickDump(unsigned int depth) const { 
	indent(depth); cout << "[Package Body Node name=" << name << " ";
	if (declared) cout << "declared";
	cout << "]" << endl;
	for (vector<StatementNode*>::const_iterator i = stmt_list.begin();
	     i != stmt_list.end();
	     i++) {
		(*i)->quickDump(depth + 1);
	}
}

void UnaryExpNode::quickDump(unsigned int depth) const { 
	indent(depth); cout << "[Unary Expression ";

	switch (op) {
		case ADD:
			cout << "+"; break;
		case SUB:
			cout << "-"; break;
		case NOT:
			cout << "not"; break;
		case ABS:
			cout << "abs"; break;
		case LITERAL:
			cout << "LITERAL"; break;

		default:
			cout << "UNKOWN OPERATOR"; break;
	}

	cout << " ]" << endl;

	if (literal) {
		indent(depth + 1); cout << *literal << endl;
	}

	if (rightOperand) {
		rightOperand->quickDump(depth + 1);
	}
}

set<Symbol *> *UnaryExpNode::evalPossibleTypes() {
	set<Symbol*> *posTypes = new set<Symbol *> ;

	if (op == ADD) {
		set<Symbol*> *posChildTypes = rightOperand->evalPossibleTypes();

		for (set<Symbol*>::const_iterator i = posChildTypes->begin();
		     i != posChildTypes->end();
		     i ++) {
			// TODO:FIXME
			// Ask Kwan here.
		}

	} else if (op == LITERAL) {
		// QP: okay, I really needed something to pass, 
		//     fix this code later if it doens't make sense
		string tmp;
		if (literal->getType() == Token::INT_LIT) {
			tmp = "__integer";
			posTypes->insert(symTab_G->getSymbolByName(tmp, true));		

		} else if (literal->getType() == Token::FLOAT_LIT) {
			tmp = "__float";
			posTypes->insert(symTab_G->getSymbolByName(tmp, true));		

		} else if (literal->getType() == Token::STRING_LIT) {
			tmp = "__string";
			posTypes->insert(symTab_G->getSymbolByName(tmp, true));		
		}
	 }

	return posTypes;
}

// TODO:FIXME
void UnaryExpNode::evalTypes(Symbol * wantedType) {
	// We don't really have to do anything here in this assigment.
}

void BinaryExpNode::quickDump(unsigned int depth) const {
	indent(depth); cout << "[Binary Expression ";

	switch (op) {
		case AND:
			cout << "AND"; break;
		case OR:
			cout << "OR"; break;
		case AND_THEN:
			cout << "AND THEN"; break;
		case OR_ELSE:
			cout << "OR ELSE"; break;
		case EQ:
			cout << "="; break;
		case SLASH_EQ:
			cout << "/="; break;
		case LT:
			cout << "<"; break;
		case LTE:
			cout << "<="; break;
		case GT:
			cout << ">"; break;
		case GTE:
			cout << ">="; break;
		case ADD:
			cout << "+"; break;
		case SUB:
			cout << "-"; break;
		case AMP:
			cout << "&"; break;
		case MULT:
			cout << "*"; break;
		case DIV:
			cout << "/"; break;
		case MOD:
			cout << "mod"; break;

		default:
			cout << "UNKOWN OPERATOR"; break;
	}

	cout << " ]" << endl;

	rightOperand->quickDump(depth + 1);
	leftOperand->quickDump(depth + 1);
}

set<Symbol *> *BinaryExpNode::evalPossibleTypes() {

	set<Symbol*> *posTypes = new set<Symbol *>;

	set<Symbol*> * posLeftChildTypes = leftOperand->evalPossibleTypes();
	set<Symbol*> * posRightChildTypes = rightOperand->evalPossibleTypes();

	Symbol * binaryOp = 0;

	string opId;
	if (op == ADD) {
		opId = "+";
	} else if (op == SUB) {
		opId = "-";
	} else if (op == AMP) {
		opId = "&";
	} else if (op == MULT) {
		opId = "*";
	} else if (op == DIV) {
		opId = "/";
	} else if (op == MOD) {
		opId = "%";
	} else if (op == EXP) {
		opId = "**";
	} else if (op == AND) {
		opId = "__and__";
	} else if (op == AND_THEN) {
		opId = "__AND_THEN__";
	} else if (op == OR_ELSE) {
		opId = "__or_else__";
	} else if (op == EQ) {
		opId = "=";
	} else if (op == SLASH_EQ) {
		opId = "/=";
	} else if (op == LT) {
		opId = "<";
	} else if (op == LTE) {
		opId = "<=";
	} else if (op == GT) {
		opId = ">";
	} else if (op == GTE) {
		opId = ">=";
	}

	// Test out all possible types.
	for (set<Symbol*>::const_iterator i = posLeftChildTypes->begin();
	     i != posLeftChildTypes->end();
	     i ++) {
		for (set<Symbol*>::const_iterator j = posRightChildTypes->begin();
		     j != posRightChildTypes->end();
		     j ++) {
			// TODO:FIXME
			// Ask Kwan here.

		}
	}

	return posTypes;
}

// TODO:FIXME
void BinaryExpNode::evalTypes(Symbol * wantedType) {
	// We don't really have to do anything here in this assigment.
}


void AssignStmtNode::quickDump(unsigned int depth) const { 
	indent(depth); cout << "[Assignstatment]" << endl;
	exp->quickDump(depth + 1);
}

// Package Body Node Definitions
PackageBodyNode::PackageBodyNode() : ExTreeNode("PackageBodyNode"), name (""), declared(false) { }
PackageBodyNode::~PackageBodyNode() { }

CompBodyNode::CompBodyNode() : ExTreeNode("CompBodyNode"){ }
CompBodyNode::~CompBodyNode() { }

DeclarationNode::DeclarationNode() : ExTreeNode("DeclarationNode") { }
DeclarationNode::~DeclarationNode() { }

void NullStmtNode::quickDump(unsigned int depth) const { 
	indent(depth); cout << "[Null]" << endl;
}

void ReturnStmtNode::quickDump(unsigned int depth) const { 
	indent(depth); cout << "[Returns]" << endl;
	if (exp) {
		exp->quickDump(depth + 1);
	}
}

void ExitStmtNode::quickDump(unsigned int depth) const { 
	indent(depth); cout << "[Exit"; 
	if (exp) {
		cout << " when....]";
		exp->quickDump(depth + 1);
	} else {
		cout << "]";
	}
	cout << endl;
}

void CaseStmtNode::quickDump(unsigned int depth) const { 
	indent(depth);  cout << "[Case Statment]" << endl;
	indent(depth + 1);  cout << "Target:" << endl;
	for (vector<CaseWhenNode *>::const_iterator i = cases.begin();
		 i != cases.end();
		 i++) {
		(*i)->quickDump(depth + 1);
	}
}
