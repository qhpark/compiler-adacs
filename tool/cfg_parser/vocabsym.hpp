#ifndef _VOCABSYM_HPP_
#define _VOCABSYM_HPP_

/**
 * Represents a Vocabulary Symbol of a CFG.
 */

#include <string>
#include <iostream>

class VocabSymbol {
   public:
      VocabSymbol(const char* buf);
      std::string getName() const;
      virtual ~VocabSymbol();
      virtual void print(std::ostream& out) const;
      virtual bool operator== (const VocabSymbol &T) const;
      virtual bool operator< (const VocabSymbol &T) const;

   protected:
      std::string name;

};

   std::ostream& operator<< (std::ostream& out, const VocabSymbol &T);

struct VocalSymbolCompare
{
  bool operator()(VocabSymbol s1, VocabSymbol s2) const
  {
    return strcmp(s1.getName().c_str(), s2.getName().c_str()) < 0;
  }
};
#endif
