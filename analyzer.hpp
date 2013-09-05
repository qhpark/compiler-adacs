#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include <vector>
#include <string>
#include "extree.hpp"

using namespace std;
class Symbol;
class SymbolTable;
class ParseTreeNode;
class TypeDescriptor;

class Analyzer {

    public:
        Analyzer();
        Analyzer(string name);
        ~Analyzer();
        vector<string>* visitIdList(ParseTreeNode *id_list, vector<string>* idents);
        void visitObjectDeclaration(ParseTreeNode *object_declar, bool forRecord, vector<Symbol*>*);     
        void visitSpecDeclareList(ParseTreeNode *spec_decl_list);
        void visitBodyDeclareList(ParseTreeNode *body_decl_list); 

		// Statements.
		void visitStmtList(ParseTreeNode *,  vector<StatementNode*> *); 
		StatementNode * visitAssignStmt(ParseTreeNode*);
		StatementNode * visitNullStmt();
		StatementNode * visitCallStmt(ParseTreeNode*);
		StatementNode * visitBlockStmt(ParseTreeNode*);
		StatementNode * visitLoopStmt(ParseTreeNode*);
		StatementNode * visitIfStmt(ParseTreeNode*);
		IfStmtNode * visitElsifStmtList(ParseTreeNode*);
		IfStmtNode * visitElseStmt(ParseTreeNode*);
		StatementNode * visitCaseStmt(ParseTreeNode*);
		void visitCaseStmt(ParseTreeNode*, CaseStmtNode *);
		void visitCaseWhenList(ParseTreeNode*, CaseStmtNode *);
		void visitChoiceList(ParseTreeNode *, CaseWhenNode *);
		void visitChoice(ParseTreeNode*, CaseWhenNode *);

		// Name
		void visitExpressionList(ParseTreeNode * , vector<set<Symbol *> *>*); 
		//void visitExpressionList(ParseTreeNode * , vector<symbolset_t> *); 
		void visitSuffixList(ParseTreeNode *node, vector<NamePacker *> &);
		NameNode * visitName(ParseTreeNode *node);

		// Expressions.
		ExpressionNode * visitExpression(ParseTreeNode *);
		ExpressionNode * visitRelation(ParseTreeNode *);
		ExpressionNode * visitSimpleExp(ParseTreeNode *);
		ExpressionNode * visitTerm(ParseTreeNode *);
		ExpressionNode * visitFactor(ParseTreeNode *);
		ExpressionNode * visitPrimary(ParseTreeNode *);
		
		// Range Expression.
		RangeNode * visitDiscreteRange(ParseTreeNode *);

        // Declaration statements
	    void visitUseClause(ParseTreeNode *use_clause);
        void visitTypeDeclaration(ParseTreeNode *type_declar);
        TypeDescriptor *visitTypeDefinition(ParseTreeNode *type_def);
        void visitSubTypeDeclaration(ParseTreeNode *subtype_declar);
        TypeDescriptor *visitSubTypeDefinition(ParseTreeNode *subtype_def);
        void visitSubProgramSpecification(ParseTreeNode *subprog_spec);

        void visitSubProgramBodyDeclaration(ParseTreeNode *body_declar);

        void visitDeclaration(ParseTreeNode *declaration);

        void visitPrivatePart(ParseTreeNode *private_part);

        void visitPrivateItem(ParseTreeNode *private_item);
        vector<string>* visitUnconstrainedRangeList(ParseTreeNode *uncons_list);
        vector<RangeNode*>* visitDiscreteRangeList(ParseTreeNode *cons_list);
        ArrayDescriptor* visitArrayDefinition(ParseTreeNode *array_node);
        EnumDescriptor*  visitEnumDefinition(ParseTreeNode *enum_node);
        RecordDescriptor* visitRecordDefinition(ParseTreeNode *record_node);
        void visitCompDeclar(ParseTreeNode *comp_decl_node, vector<Symbol*>* comps);
        void visitVariantPart(ParseTreeNode *variant_node, vector<Variant*>* vars);
        Symbol* visitTypeOrSubtype(ParseTreeNode *node);        

        PackageBodyNode* visitPackageBody(ParseTreeNode *node);

        void visitPackageDeclaration(ParseTreeNode *node);

        PackageBodyNode* visitCompilationUnit(ParseTreeNode * node);

        void visitCompilationUnits(ParseTreeNode * node, vector<PackageBodyNode*> &pkg_list); 
        
        void printSymbolTable() const;

		void addUse(Symbol * symbol, unsigned int line, Extree * treenode);
    private:
        SymbolTable *symTab;
		string filename;
		vector<SymbolUse> usage;
};
    

#endif
