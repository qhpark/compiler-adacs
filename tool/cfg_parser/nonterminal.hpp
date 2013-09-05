#ifndef _NONTERMINAL_HPP_
#define _NONTERMINAL_HPP_

/**
 * Represents a NonTerminal of a CFG.
 */

#include "vocabsym.hpp"
#include <string>
#include <iostream>

class NonTerminal : public VocabSymbol  {
   
   public:
      NonTerminal(const char* buf);
      ~NonTerminal();
      void print(std::ostream &out) const;

};
#endif
