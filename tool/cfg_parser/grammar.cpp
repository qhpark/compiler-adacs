#include <iostream>
using namespace std;

#include "grammar.hpp"
#include "nonterminal.hpp"
#include "sententialform.hpp"
#include "terminal.hpp"
#include <typeinfo>
#include <string>
using namespace std;

// Create a grammar with given start symbol
Grammar::Grammar(NonTerminal &start) : start(start) {
   nonterminals.insert(start);
}

// Deallocate a grammar
// Make sure to go through productions
Grammar::~Grammar() {
   
}

// Add a production rule to the grammar. 
// 
// Memory Ownership:
// Grammar owns rhs and is responsible for it's deallocation.
void Grammar::AddProduction(NonTerminal &nt, SententialForm* rhs) {
      // Allocate new node
      RuleNode* newNode = new RuleNode();
      newNode->next = NULL;
      newNode->rule = rhs;

      map<NonTerminal, RuleNode* >::iterator iter = productions.find(nt);
      if (iter == productions.end()) {
         // Add first production for nt
         productions[nt] = newNode;
      } else {
         // Append newNode to list of productions
         RuleNode* pos = productions[nt];
         if(pos != NULL) {
            while(pos->next != NULL) { pos = pos->next; }
            pos->next =  newNode;
         } else {
            // existing epsilon production
            productions[nt] = newNode;
         }
      } // if
}

// Add a nonterminal to the grammar
void Grammar::AddNonTerminal(NonTerminal &nt) {
   nonterminals.insert(nt);
}

// Add a terminal to the grammar
void Grammar::AddTerminal(Terminal &t) {
   terminals.insert(t);
}

// Overload the stream output operator
ostream& operator<<( ostream &out, const Grammar &G) {
   out<<"Start Symbol: "<<(G.start);
   return out;
}

// Return the map of grammar productions
map<NonTerminal, Grammar::RuleNode* >& Grammar::Productions() {
   return productions;
}

// Return the set of non-terminals in the grammar
set<NonTerminal>& Grammar::NonTerminals() {
   return nonterminals;
}

// Return the set of terminals in the grammar
set<Terminal> & Grammar::Terminals() {
   return terminals;
}

// Return true, iff the given NonTerminal is the start symbol
bool Grammar::IsStart(const NonTerminal &nt) const {
   return nt== start;
}

// Debug output
void Grammar::PrintProductions() {
   map<NonTerminal, Grammar::RuleNode*>::iterator iter = productions.begin();
   while(iter != productions.end()) {
      RuleNode* node = (*iter).second;
      cout<<(*iter).first<<" ";
      while(node != NULL) {
         cout<<"-> "<<*(node->rule);
         node = node->next;
      }
      cout<<endl;
      iter++;
   }
}

// Debug output 
void Grammar::PrintNonTerminals() const {
   set<NonTerminal>::iterator iter = nonterminals.begin();
   cout<<"\nNonTerminals\n[";
   for(; iter != nonterminals.end(); iter++ ) {
      cout<<*iter<<endl;
   }
   cout<<"]"<<endl;

}

// Debug output
void Grammar::PrintTerminals() const {
   set<Terminal>::iterator iter = terminals.begin();

   cout<<"\nTerminals\n[";
   for(; iter != terminals.end(); iter++ ) {
      cout<<*iter<<endl;
   }
   cout<<"]"<<endl;
}

// Dump a set of terminals to stdout.
void Grammar::printTerminalSet(std::set<Terminal> & tSet) {
	for (set<Terminal>::const_iterator i = tSet.begin();
	     i != tSet.end(); i++) {
		cout << *i << " ";
	}
}

