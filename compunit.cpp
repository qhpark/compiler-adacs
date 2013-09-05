#include "compunit.hpp"
#include <string>
#include <iostream>
#include "debug.hpp"

CompileUnit::CompileUnit(std::string name) 
  : filename(name), state(SOURCE), tkStream(0), astRoot(0) {

      analyzer = new Analyzer(name);
}

CompileUnit::~CompileUnit() {
    delete analyzer;

	// Must delete tokens here!
	switch (state) {
		case SCANNED:
			for (std::list<Token *>::const_iterator i = tkStream->begin();
			     i != tkStream->end(); i++) {
				delete *i;
			}
			break;
		case PARSED: 
			// Delete the parse tree here.
			break;
	
		defaut:
			// Don't have to do much here.
			break;
	}
}

const std::string & CompileUnit::getFileName() {
	return filename;
}

std::ostream& operator<<( std::ostream &out, const CompileUnit &cu) {
	out << "CU name=" << cu.filename << " state=";
	switch (cu.state) {
		case CompileUnit::SOURCE:
			out << "SOURCE ";
			break;
		case CompileUnit::SCANNED:
			out << "SCANNED ";
			break;
		case CompileUnit::PARSED:
			out << "PARSED ";
			break;
		case CompileUnit::CODEGENED:
			out << "CODEGENED ";
			break;
	}
}

void CompileUnit::printTokenStream() const {
	for (std::list<Token *>::const_iterator i = tkStream->begin();
	     i != tkStream->end(); i++) {
		std::cout <<  *(*i) << std::endl;
	}
}

void CompileUnit::printParseTree() const {
	std::cout << std::endl;
	parseTreeRoot->quickDump();
}


void CompileUnit::printSymbolTable() const {
	std::cout << std::endl;
    analyzer->printSymbolTable();
}
