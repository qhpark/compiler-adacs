#include "vocabsym.hpp"
#include <string.h>

VocabSymbol::VocabSymbol(const char* buf) {
   name = buf;
}

VocabSymbol::~VocabSymbol() {

}

bool VocabSymbol::operator== (const VocabSymbol &T) const {
   int cmp = strcmp(this->name.c_str(), T.name.c_str());
   return cmp == 0;
}

bool VocabSymbol::operator< (const VocabSymbol &T) const {
   int cmp = strcmp(this->name.c_str(), T.name.c_str());
   return cmp < 0;
}

// Accessor.
std::string VocabSymbol::getName() const {
   return name;
}

// Output the symbol name
void VocabSymbol::print(std::ostream& out) const {
 out<<name;  
}

// Call the appropriate virtual print method 
std::ostream& operator<< (std::ostream &out, const VocabSymbol &T) {
   T.print(out);
   return out;
}
