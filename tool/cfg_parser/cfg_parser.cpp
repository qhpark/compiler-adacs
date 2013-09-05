#include "cfg_parser.hpp"
#include "grammar.hpp"
#include "nonterminal.hpp"
#include "sententialform.hpp"
#include <stdlib.h>

// Constructor
CfgParser::CfgParser(string &name) : sFilename(name), BufSize(512)
{
   pos = 0;
   cfg = NULL;
   buffer = new char[BufSize];
   
   // Initialize buffer to nulls
   for(int i = 0; i < BufSize; i++) {
      buffer[i] = '\0';
   }

}

// Destructor
CfgParser::~CfgParser() {
   delete cfg;
   delete[] buffer;
}

// Implement Cfg parser as a simple finite machine
void CfgParser::parseCfg() {
   // Open file stream for reading
   istream *in = new ifstream(sFilename.c_str());
   // Don't skip over whitespace characters
   *in>>noskipws;
   
   // Initialize state variables
   char ch;
   state s = NEWPRODUCTION;
   NonTerminal *lhs = (NonTerminal*)0;
   SententialForm *rule = (SententialForm*)0;

   while(true) {
      // Process input character-by-character
      *in>>ch;
      if(in->eof()) { break; }

      switch(s) {
         case NEWPRODUCTION:
            if(ch == '<') {
               pos = 0;
               buffer[pos] = '\0';
               s = LHS;
            }
            break;

         case LHS:
            // Read name of NonTerminal on Lhs of rule
            if(ch == '>') {
               buffer[pos] = '\0';
               pos = 0;

               // Record NonTerminal with CFG
               lhs = new NonTerminal(buffer);
               if(cfg == (Grammar*)0) { 
                  cfg = new Grammar(*lhs);
               }else {
                  cfg->AddNonTerminal(*lhs);
               }

               // Advance input to "->" marker
               bool found = lookForChar(*in, '-');

               if(found) {
                  s = ARROW;
               } else {
                  printAndExit("Expected \"->\" not found.");
               } // if

            } else {
               buffer[pos++] = ch;
            }
            break;

         case ARROW:
            if(ch == '>') {
               s = RHS;
               // Make sure we have a valid Rhs var
               // before we start adding symbols to it
               rule = new SententialForm();
            }
            break;

         case SHORTHAND:
            // Handles cases where the shorthand notation for 
            // specifying multiple rules for a single non-terminal is used.
            if(ch == '>') {
               s = RHS;
               if(rule != (SententialForm*)0) {
                  cfg->AddProduction(*lhs, rule);
               }
               rule = new SententialForm();
            }
            break;

         case RHS:
            // Handles the Rhs of a production rule
            // Further goes to NONTERMINAL and TERMINAL states
            // to handle actual symbols.
            if( ch == ' ') {
               // Eat all the spaces
            } else if(ch == '-') {
               if(in->peek() == '>') {
                  s = ARROW;
               } else {
                  // subtract operator
               buffer[pos++] = ch;
               s = TERMINAL;

               }
            } else if(ch == '<') {
               if(isalpha(in->peek())) {
               s = NONTERMINAL;
               } else {
                  // relational operator
                buffer[pos++] = ch;
               s = TERMINAL;

               }
               
            } else if(ch == '\n') {
               *in>>ch;
               if(ch == '\n') {
                  // A blank line delimits production rules for
                  // different non-terminals
                  s = NEWPRODUCTION;
                  if(rule != (SententialForm*)0) {
                     cfg->AddProduction(*lhs, rule);
                  }
               } else {
                  // A production rule that spans multiple lines
                  // or shorthand notation being used. Still the
                  // same Lhs.
                  ch = consumeSpaces(*in);
                  if(ch == '-') {
                     s = SHORTHAND;
                  } else if(ch == '<') {
                     s = NONTERMINAL;
                  } else {
                     s = TERMINAL; 
                     buffer[pos++] = ch;
                  }
               }
            } else {
               buffer[pos++] = ch;
               s = TERMINAL;
            }
            break;

         case NONTERMINAL:
            // Read the name of a non-terminal on the Rhs
            if (ch == '>') {
               s = RHS;
               buffer[pos] = '\0';
               pos = 0;

               // Add NonTerminal to set of non-terminals 
               // and append to current rule
               NonTerminal *nt = new NonTerminal(buffer);
               cfg->AddNonTerminal(*nt);
               rule->AddSymbol(nt);
            } else {
               buffer[pos++] = ch;
            }
            break;

         case TERMINAL:
            // Read the name of a terminal on the Rhs
            if(ch == ' ' || ch == '\n') {
               if(ch == '\n') { 
                  // Put back newline so RHS state can detect blank lines
                  in->putback(ch); 
               }
               s = RHS;
               buffer[pos] = '\0';
               pos = 0;
               // Add Terminal to set of terminals and 
               // append to current rule
               Terminal *t = new Terminal(buffer);
               cfg->AddTerminal(*t);
               rule->AddSymbol(t);
            } else {
               buffer[pos++] = ch;
            }
            break;
      };
   }

   delete in;
   // Output the CFG
   cfg->PrintProductions();
}


// Consume all spaces up to and including first 
// non-space char.
// Return the first non-space char.
char CfgParser::consumeSpaces(istream &s) {
   char ch;
   while(true) {
      s>>ch;
      if(s.eof()) { break; }
      if(ch != ' ') { break;}
   }
   return ch;
}

// Scan input until ch is found or end-of-file reached.
// Return true iff ch is found.
bool CfgParser::lookForChar(istream &s, char ch) {
   char tmp;
   while(true) {
      s>>tmp;
      if(s.eof()) { return false; }
      if(tmp == ch) { return true; }
   }
}

// Print last words and terminate
void CfgParser::printAndExit(char *msg) {
   cout<<"\n"<<msg<<endl;
   exit(-1);
}

// Get the result.
Grammar * CfgParser::getCfg() {
   return cfg;
}

// Driver routine
int main(unsigned int argc, char** argv) {
   if(argc != 2) { 
      cerr<<"Usage: " << argv[0]  << " <cfg filename>" <<endl;
      exit(-1);
   };
   string str(argv[1]);
   CfgParser parser(str);
   parser.parseCfg();

   // Create tables.
   parser.getCfg()->buildNullable();
   parser.getCfg()->printNullable();

   parser.getCfg()->buildFirst();
   parser.getCfg()->printFirst();

   parser.getCfg()->buildFollow();
   parser.getCfg()->printFollow();

   return 0;
};
