#ifndef _CFG_PARSER_HPP_
#define _CFG_PARSER_HPP_
/**
 * Parses CFG grammars written in standard notation.
 *
 */
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Grammar;

class CfgParser {
   public:
      // Parser states
      enum state { 
         NEWPRODUCTION, 
         SHORTHAND, 
         NONTERMINAL, 
         TERMINAL, 
         ARROW, 
         RHS, 
         LHS
         } ;
      
      // Constructor
      CfgParser(string &name);

      // Destructor
      ~CfgParser();

      // Main parsing routine
      void parseCfg();

      // Delivers the good stuff
      Grammar * getCfg();
      
   private:
      // Variables for buffering input
      unsigned int pos;
      const unsigned int BufSize;
      char *buffer;
      // Name of CFG
      string& sFilename;
      // The CFG
      Grammar *cfg;
      
      void printAndExit(char* msg);
      bool lookForChar(istream &s, char ch);
      char consumeSpaces(istream &s);
};


#endif
