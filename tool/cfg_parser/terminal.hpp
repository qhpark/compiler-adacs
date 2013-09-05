#ifndef _TERMINAL_HPP_
#define _TERMINAL_HPP_

/**
 * Represents a Terminal of a CFG.
 */
#include "vocabsym.hpp"
#include <string>
#include <iostream>

class Terminal : public VocabSymbol {
   public:
      Terminal(const char* buf);
      ~Terminal();
      void print(std::ostream &out) const;
};

#endif
