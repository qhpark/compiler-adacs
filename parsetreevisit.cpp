#include <vector>
#include <iostream>
#include <string>
#include "compunit.hpp"
#include "debug.hpp"
#include "parsetree.hpp"
#include "extree.hpp"

using namespace std;


bool CompileUnit::parseTreeVisit() {

	ASSERT(state == PARSED, "Can only tranverse if the the file has been parsed");
	// Indeed.

	ASSERT(parseTreeRoot, "Can you transverse a parse tree if the tree exists!");

    // At the top-level we have a list of packages (compilation units) 
    // TODO: pragma_list
	vector<PackageBodyNode *> pkg_list;
	analyzer->visitCompilationUnits(parseTreeRoot, pkg_list);
    astRoot = new CompilationsNode(pkg_list); 
	return true;
}



void CompileUnit::printAST() const {
    if(astRoot) {
        //cout<<*astRoot;    
		astRoot->quickDump();
    }
}
