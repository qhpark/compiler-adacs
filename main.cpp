#include <iostream>
#include <list>
#include <string>
#include "debug.hpp"
#include "compunit.hpp"
#include "timer.hpp"

#include "unistd.h"
#include "parsetree.hpp"

Debug debugOpt;
Timer timer;

// Parse Flags
void parseArgs(char**, unsigned int, std::list<std::string> *);

// Test Symbol Table

// Main
int main(unsigned int argc, char** arg) {

	// Read in command parameter.
	std::list<std::string> * srcFiles = new std::list<std::string>();
	parseArgs(arg, argc, srcFiles);
	if (debugOpt.inDebug("cmdarg")) {
		std::cout << debugOpt << std::endl;
		std::cout << "srcFiles={";
		for (std::list<std::string>::const_iterator i = srcFiles->begin();
			 i != srcFiles->end(); i++) {
			std::cout << *i << " ";
		}
		std::cout << "}" << std::endl;
	}
    
    // Self-Tests
	// Run some tests on Symbol Table
	if (debugOpt.inDebug("stselftest")) {
        SymbolTable st;
        Symbol* a = new Symbol(Symbol::VAR);
        a->setName("A");
        Symbol* a1 = new Symbol(Symbol::BLOCK);
        a1->setName("A");

        Symbol* b = new Symbol(Symbol::VAR);
        b->setName("B");

        st.AddSymbol(a);
        Symbol* sym = st.getSymbolByName("A", false);
        cout<<sym->getSymbolType()<<endl;
        cout<<"open scope and override A"<<endl;
        st.OpenScope(); 
        
        sym = st.getSymbolByName("B", false);
        cout<<"has B? "<<(sym != 0)<<endl;
        st.AddSymbol(b);
        sym = st.getSymbolByName("B", false);
        cout<<"has B? "<<(sym != 0)<<endl;
        
        st.AddSymbol(a1);
        sym = st.getSymbolByName("A", false);
        cout<<sym->getSymbolType()<<endl;
        cout<<"close scope"<<endl;
        st.CloseScope(); 
        sym = st.getSymbolByName("A", false);
        cout<<sym->getSymbolType()<<endl;
        Symbol* p1 = new Symbol(Symbol::TYPE);
        p1->setTypeDescriptor(new ProcDescriptor());
        p1->setName("P1");
        Symbol* p2 = new Symbol(Symbol::TYPE);
        p2->setTypeDescriptor(new ProcDescriptor());
        p2->setName("P1");
        st.AddSymbol(p1);
        st.AddSymbol(p2);

        return 0;
    }

	if (srcFiles->empty()) {
		std::cerr << "Missing a source file" << endl;
		return 1;
	}
	
	for (std::list<std::string>::const_iterator i = srcFiles->begin();
			i != srcFiles->end(); i++) {
		if (access(i->c_str(),R_OK)) {
			std::cerr << *i << " is not readable." << endl;
			return 1;
		}
	}


	// Create the compile unit for each of the source file.
	std::list<CompileUnit*> * cuList = new std::list<CompileUnit *>();
	for (std::list<std::string>::const_iterator i = srcFiles->begin();
	     i != srcFiles->end(); i++) {
		cuList->push_back(new CompileUnit(*i));
	}


	// Scanning for each of the compile unit.
	TimerEntryIndex scannerTimer = timer.addEntry("Scanner/Tokenizer");
	timer.start(scannerTimer);
	for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
	     i != cuList->end(); i++) {
		if (!(*i)->scan()) {
			std::cerr << "Scanning of " << (*i)->getFileName() << " failed.";
			std::cerr << " Bailing out. " << std::endl;
			return 1;
		}
	}
	timer.stop(scannerTimer);

	// We no longer need the file names since they are stored
	// in the compile unit.
	delete srcFiles;

	if (debugOpt.inDebug("dumptkonly")) {
		for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
		     i != cuList->end(); i++) {
			std::cout << std::endl;
			std::cout << "Dumping Token Stream: " << (*i)->getFileName() << std::endl;
			std::cout << "----------------------------------" << std::endl;
			(*i)->printTokenStream();
			std::cout << "----------------------------------" << std::endl;
			std::cout << std::endl;
		}
		return 0;
	}

	// parsening for each of the compile unit.
	TimerEntryIndex parserTimer = timer.addEntry("Parser");
	timer.start(parserTimer);
	for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
	     i != cuList->end(); i++) {

		if (!(*i)->parse()) {
			std::cerr << "Parsing of " << (*i)->getFileName() << " failed.";
			std::cerr << " Bailing out. " << std::endl;

			return 1;
		}
	}
	timer.stop(parserTimer);

	// See if we 
	if (debugOpt.inDebug("dumpptonly")) {
		for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
		     i != cuList->end(); i++) {
			std::cout << std::endl;
			std::cout << "Dumping Parse Tree: " << (*i)->getFileName() << std::endl;
			std::cout << "----------------------------------" << std::endl;
			(*i)->printParseTree();
			std::cout << "----------------------------------" << std::endl;
			std::cout << std::endl;
		}
		return 0;
	}

	// Info gathering for each of the compile unit.
	TimerEntryIndex gatherTimer = timer.addEntry("Gather / Form IR");
	timer.start(gatherTimer);
	for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
	     i != cuList->end(); i++) {

		if (!(*i)->parseTreeVisit()) {
			std::cerr << "Gathering of " << (*i)->getFileName() << " failed.";
			std::cerr << " Bailing out. " << std::endl;
			return 1;
		}
	}
	timer.stop(gatherTimer);

    // Dump text representation of AST
    if (debugOpt.inDebug("dumpastonly")) {
        for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
                i != cuList->end(); i++) {
            std::cout << std::endl;
            std::cout << "Dumping AST: " << (*i)->getFileName() << std::endl;
            std::cout << "----------------------------------" << std::endl;

            (*i)->printAST();

			std::cout << "----------------------------------" << std::endl;
			std::cout << std::endl;
        } // for
		return 0;
    }

    if (debugOpt.inDebug("timing")) {
        std::cout << timer << std::endl;
    }
    /*
	if (debugOpt.inDebug("dumpstonly")) {
		for (std::list<CompileUnit *>::const_iterator i = cuList->begin();
		     i != cuList->end(); i++) {
			std::cout << std::endl;
			std::cout << "Dumping Symbol Table: " << (*i)->getFileName() << std::endl;
			std::cout << "----------------------------------" << std::endl;
			(*i)->printSymbolTable();
			std::cout << "----------------------------------" << std::endl;
			std::cout << std::endl;
		}
		return 0;
	}
*/
    
}



// Parse Flags
void parseArgs(char ** arg, unsigned int argc, std::list<std::string> * src) {
    for (unsigned int curArg = 1; curArg < argc; ++curArg) {
        if (arg[curArg][0] == '-') {
            char option = arg[curArg][1];
            switch (option) {
                case 'D':
                    debugOpt.addDebug(&(arg[curArg][2]));
                    break;
            }
        } else {
            src->push_back(arg[curArg]);
        }
    }
}
