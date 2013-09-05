#ifndef _COMPUNIT_HPP_
#define _COMPUNIT_HPP_

#include <iostream>
#include <string>
#include <list>
#include "token.hpp"
#include "parsetree.hpp"
#include "extree.hpp"
#include "symtab.hpp"
#include "analyzer.hpp"
#include <stack>
#include <list>
#include <vector>

// Basic Abstruction of a single file to be compiled.
class CompileUnit {
	friend std::ostream& operator<<( std::ostream &out, const CompileUnit &C);
	public:
		enum State { SOURCE, SCANNED, PARSED, CODEGENED };
		CompileUnit(std::string name);
		~CompileUnit();

		// Start the scanning.
		bool scan();

		// Start the parsing process.
		bool parse();

		// After parsing the file, traverse the parse tree.
		// to collect information.
		bool parseTreeVisit();

		// Dump the tokens to stdout.
		void printTokenStream() const;

		// Dump the parse tree.
		void printParseTree() const;

        // Dump the AST.
        void printAST() const;

        // Dump the symbol table.
		void printSymbolTable() const;
        
		// Return the name of the file.
		const std::string & getFileName();

		typedef unsigned int ParserState;

	private:
		std::string filename;
		State state;

		// Pointer to the last of all tokens.
		std::list<Token *> * tkStream;
	
		// Parse tree.
		ParseTreeNode * parseTreeRoot;

        // AST tree.
        ExTreeNode *astRoot;

        // Parse Tree Analyzer
        Analyzer *analyzer;
        
		// For parser:
		// Tries to keep parsing but no making any trees.
		// Return 0 if there is no error.
		// Return the token that caused the error.
		Token * 
		   recover(std::stack<ParserState> & states, 
		           std::list<ParseTreeNode *> & inputstream,
		           unsigned int removePrefix,
		           unsigned int removeSuffix,
		           bool debugMsg);

		void visitCompilationUnits(ParseTreeNode * node, std::vector<PackageBodyNode*> &list);
};

#endif
