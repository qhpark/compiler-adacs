#ifndef _EXTREE_HPP_
#define _EXTREE_HPP_

#include <vector>
#include <string>
#include <iostream>
#include <set>
#include "symtab.hpp"

using namespace std;


#define indent(depth) \
	for (int ii = 0; ii < depth; ii++) { \
		cout << " "; \
	} 
// Representation of a parse tree node.

class Token;

// Intermediate Representation of the program. 
class ExTreeNode {
    friend std::ostream& operator<<( std::ostream &out, const ExTreeNode &T);

	public:
		ExTreeNode(std::string name);
		~ExTreeNode();

		virtual void quickDump() const;
		virtual void quickDump(unsigned int depth) const;

	
    private:
        std::string nodename;
};

class StatementNode;
class DeclarationNode;
class ExpressionNode;
class RangeNode;
class NameNode;

class PackageBodyNode : public ExTreeNode {

	public:
		PackageBodyNode();
		~PackageBodyNode();
        vector<StatementNode*> stmt_list;
		string name;
		bool declared;

		virtual void quickDump(unsigned int depth) const;
};

class CompilationsNode: public ExTreeNode {
    public:
        CompilationsNode(vector<PackageBodyNode*> pkgs) : ExTreeNode("Compilations"), pkg_list(pkgs) {}
        ~CompilationsNode() { }
		virtual void quickDump(unsigned int depth) const { 
			for (vector<PackageBodyNode*>::const_iterator i = pkg_list.begin();
			     i != pkg_list.end();
			     i++) {
				(*i)->quickDump(depth + 1);
			}
		}
   private:
        vector<PackageBodyNode*> pkg_list;
};

class CompSpecNode : public ExTreeNode {
	public:
		CompSpecNode();
		~CompSpecNode();

	private:
		Token * packageName;
		
};

class StatementNode : public ExTreeNode {
	public:
		StatementNode() : ExTreeNode ("Statemnt") { }
		~StatementNode() { }
		virtual void quickDump(unsigned int depth) const { 
			indent(depth); cout << "[statment]" << endl;
		}

};

class AssignStmtNode : public StatementNode {
	public:
		AssignStmtNode() : exp(0) { }
		~AssignStmtNode() { }

		virtual void quickDump(unsigned int depth) const; 
		// Name:

		// Assigned to:

		// Expression:
		ExpressionNode * exp;
};

class BlockStmtNode : public StatementNode {
	public:
		BlockStmtNode() { }
		~BlockStmtNode() { }
        vector<StatementNode*> stmt_list;
		virtual void quickDump(unsigned int depth) const { 
			indent(depth); cout << "[Block]" << endl;
			for (vector<StatementNode*>::const_iterator i = stmt_list.begin();
			     i != stmt_list.end();
			     i++) {
				(*i)->quickDump(depth + 1);
			}
		}
};


class LoopStmtNode : public StatementNode {
	public:

		enum Type { INFINITE, WHILE, FOR };
		Type type;

		LoopStmtNode() : type (INFINITE), 
		                 whileCond(0), reverse(false), range(0) { }

		~LoopStmtNode() { }
        vector<StatementNode*> stmt_list;


		// While.
		ExpressionNode * whileCond;

		// For.
		bool reverse;
		RangeNode * range;

		virtual void quickDump(unsigned int depth) const { 
			indent(depth); cout << "[Loop]" << endl;
			for (vector<StatementNode*>::const_iterator i = stmt_list.begin();
			     i != stmt_list.end();
			     i++) {
				(*i)->quickDump(depth + 1);
			}
		}
};

class ExpressionNode : public ExTreeNode {


	public:
		ExpressionNode() : ExTreeNode("Expression"), parentExpression(0) { }
		~ExpressionNode() { }

		// For passing stuff up.
		ExpressionNode * parentExpression;

		// Sythesize up.
		virtual set<Symbol *> *evalPossibleTypes() {
			return new set<Symbol*>;
		}

		// Inhereitned down.
		virtual void evalTypes(Symbol * wantedType) {

		}

};

class UnaryExpNode : public ExpressionNode {
	public:
		enum OpType { 
			UNKNOWN, ADD, SUB, NOT, ABS, LITERAL, NAME
		};

		UnaryExpNode() : op(UNKNOWN), rightOperand(0), literal(0), name(0) { }
		~UnaryExpNode() { }

		virtual void quickDump(unsigned int depth) const;

		// Unary operation.
		OpType op;
		ExpressionNode * rightOperand;

		// Literal
		Token * literal;

		// Name / Symbol.
		NameNode * name;

		// Function call

		// Type checking.
		virtual set<Symbol *> *evalPossibleTypes();
		virtual void evalTypes(Symbol * wantedType);
};

class BinaryExpNode : public ExpressionNode {
	public:
		enum OpType { 
			UNKNOWN, ADD, SUB, AMP, MULT, DIV, MOD, EXP,
			AND, OR, AND_THEN, OR_ELSE,
			EQ, SLASH_EQ, LT, LTE, GT, GTE
		};

		BinaryExpNode() : op(UNKNOWN), leftOperand(0), rightOperand(0) { }
		~BinaryExpNode() { }

		virtual void quickDump(unsigned int depth) const;

		OpType op;
		ExpressionNode * leftOperand;
		ExpressionNode * rightOperand;

		// Type checking.
		virtual set<Symbol *> *evalPossibleTypes();
		virtual void evalTypes(Symbol * wantedType);

};

class RangeNode : public ExTreeNode {

	public:
		RangeNode() : ExTreeNode("Range") {}
		~RangeNode() {}

};

class ExpressionRangeNode : public RangeNode {

	public:
		ExpressionRangeNode() : lower(0), upper(0) {}
		~ExpressionRangeNode() {}

		ExpressionNode * lower;
		ExpressionNode * upper;
};

class CompBodyNode : public ExTreeNode {
	public:
		CompBodyNode();
		~CompBodyNode();

	private:
		Token * packageName;
		std::vector<StatementNode *> statements;
		std::vector<DeclarationNode *> declarations;
		
};



class DeclarationNode : public ExTreeNode {
	public:
		DeclarationNode();
		~DeclarationNode();

};

class NullStmtNode : public StatementNode {
	public:
		NullStmtNode() {} 
		~NullStmtNode() {}
	virtual void quickDump(unsigned int depth) const; 
};

class ReturnStmtNode : public StatementNode {
	public:
		ReturnStmtNode() : exp(0) { } 
		~ReturnStmtNode() {}

		ExpressionNode * exp;

		virtual void quickDump(unsigned int depth) const; 
};

class ExitStmtNode : public StatementNode {
	public:
		ExitStmtNode() : exp(0) { } 
		~ExitStmtNode() {}

		ExpressionNode * exp;

		virtual void quickDump(unsigned int depth) const; 
};


/*
 * Not really going to suppose something like this are we?
 */
class RaiseStmtNode : public StatementNode {
	public:
		RaiseStmtNode() : id("UNKNOWN EXCEPTION") {} 
		~RaiseStmtNode() {}

		string id;	

		virtual void quickDump(unsigned int depth) const {
			indent(depth);  cout << "[Raise " << id << endl;
		}
};

class IfStmtNode : public StatementNode {
	public:
		IfStmtNode() : type(IF), cond(0), elsifStmt(0), elseStmt(0) {} 
		~IfStmtNode() {}

		enum Type { IF, ELSIF, ELSE };
		Type type;
		
		// If (condition) blah blah blah....
		ExpressionNode * cond;

		// Elsif
		IfStmtNode * elsifStmt;
		IfStmtNode * elseStmt;

        vector<StatementNode*> stmt_list;
	
		virtual void quickDump(unsigned int depth) const {
			indent(depth);  cout << "[If " << endl;
		}
};

class CaseWhenNode;
class CaseStmtNode : public StatementNode {
	public:
		CaseStmtNode() : target(0) {}
		~CaseStmtNode() {}

		ExpressionNode * target;

		vector<CaseWhenNode *> cases;
		
		virtual void quickDump(unsigned int depth) const;
};

class CaseWhenNode : public ExTreeNode {
	public:
		CaseWhenNode() : ExTreeNode("Case When"), parent(0) {}
		~CaseWhenNode() {}

		CaseStmtNode * parent;

		vector<ExpressionRangeNode *> ranges;
		vector<ExpressionNode *> conds;
		vector<StatementNode*> stmt_list;

};

class NameNode : public ExTreeNode {
	public:

		NameNode(string id): ExTreeNode("name"), all_access(false), id(id) {} 
		~NameNode() {}
	
		string id;	
		vector<NamePacker*> namelist;
		Symbol *sym;
		bool all_access;
		
	virtual void quickDump(unsigned int depth) const {
		indent(depth); 
		cout << "[NameNode: " << id << ", All: ";
		if (all_access) cout << "true";
		else cout << "false";
		cout << "]" << endl;
		for (vector<NamePacker*>::const_iterator i = namelist.begin();
			 i != namelist.end();
			 i++) {
			(*i)->quickDump(depth + 1);
		}
	}
};

class CallStmtNode : public StatementNode {
	public:
		CallStmtNode() {} 
		~CallStmtNode() {}
	virtual void quickDump(unsigned int depth) const {
		indent(depth); cout << "[CallStmt]" << endl;
		name->quickDump(depth + 1);
	}
	NameNode *name; 
};

class AggComponentNode : public ExTreeNode {

	// If this is empty this mean it is an other.
	vector<ExpressionNode *> aggs_choice;
	ExpressionNode * value;

	public:
		AggComponentNode() : ExTreeNode("AggComponentNode") {}
		~AggComponentNode() {}
};

class AggregateNode : public ExTreeNode {

	NameNode * typeCast;
	vector<AggComponentNode *> com_list;

	public:
		AggregateNode() : ExTreeNode("AggregateNode") {}
		~AggregateNode() {}
};
#endif
