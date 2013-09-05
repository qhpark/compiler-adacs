#include "nonterminal.hpp"
#include <string.h>

NonTerminal::NonTerminal(const char* buf) : VocabSymbol(buf) {
}

NonTerminal::~NonTerminal() {

}

void NonTerminal::print(std::ostream &out) const {
   out<<"<"<<name<<">";
}
