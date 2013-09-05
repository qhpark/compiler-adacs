#ifndef _GRAMMAR_HPP_
#define _GRAMMAR_HPP_

/**
 * Represents a CFG.
 */ 

#include <list>
#include <map>
#include <set>
#include "terminal.hpp"
#include "nonterminal.hpp"
class SententialForm;

class Grammar {

   friend std::ostream& operator<<( std::ostream &out, const Grammar &G);

   public:
   struct RuleNode {
      RuleNode* next;
      SententialForm* rule;
   };

   // Create a grammar with given start symbol
   Grammar(NonTerminal &start);

   // Deallocate a grammar
   ~Grammar();

   // Add a production rule to the grammar    
   void AddProduction(NonTerminal &nt, SententialForm* rhs);

   // Add a nonterminal to the grammar
   void AddNonTerminal(NonTerminal &nt);

   // Add a terminal to the grammar
   void AddTerminal(Terminal &t);

   std::map<NonTerminal, RuleNode* > &Productions() ;
   std::set<NonTerminal> &NonTerminals() ;
   std::set<Terminal> &Terminals() ;

   bool IsStart(const NonTerminal &nt) const;
   void PrintNonTerminals() const;
   void PrintTerminals() const;
   void PrintProductions() ;

   // Build the whole nullable table.
   void buildNullable();

   // Print what we got.
   void printNullable();

   // Build the whole first table.
   void buildFirst();

   // Print what we got.
   void printFirst();

   // Build the whole first able.
   void buildFollow();

   // Print what we got.
   void printFollow();

   typedef std::set<Terminal> TermSet;


   private:
   std::map<NonTerminal,  RuleNode*> productions;
   std::set<NonTerminal> nonterminals;
   std::set<Terminal> terminals;

   // Keep track of what has been done.
   bool builtNullable;
   bool builtFirst;
   bool builtFollow;

   // Look up tables.
   std::map<NonTerminal, bool, VocalSymbolCompare> nullable;
   std::map<NonTerminal, TermSet, VocalSymbolCompare> first;
   std::map<NonTerminal, TermSet, VocalSymbolCompare> follow;

   NonTerminal &start;

   // Check if a rule is nullable.
   // Note this can be very incomplete if the whole nullable table
   // has not been generated.
   // Basically the result is based on the current state of the
   // nullable table.
   //
   // Return non zero when it is nullable.
   bool checkNullable(SententialForm* rule);

   // return true if there are any changes.
   bool calculateFirstSet(NonTerminal nt, SententialForm * rule);
   bool calculateFollowSet(NonTerminal nt, SententialForm * rule);

   void printTerminalSet(std::set<Terminal> &);
};

#endif
