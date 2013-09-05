#include "terminal.hpp"
#include <string.h>

Terminal::Terminal(const char* buf) : VocabSymbol(buf) {
   
}

Terminal::~Terminal() {

}

void Terminal::print(std::ostream &out) const {
   out<<name;
}
