#!/usr/bin/python

# Utility to generate parse tables for LR(1) grammar.

import sys
import string
import os.path

from operator import itemgetter

# Some global variables to manage ids for
# states and symbols
state_num = 0
itemNum = 0
term_num = 0
nonterm_num = 0
rule_num = 0
term_id = {}
nonterm_id = {}
rule_id = {}

#==============================================================================
# Grammar reads in a CFG and builds an object model that can be manipulated.
# First, Nullable, and Follow sets are also computed for each NonTerminal.
class Grammar:
   def SelfTest(self):
      self.PrintNonTerminals()
      self.PrintProductions()
      self.BuildNullable()
      self.PrintNullable()
      self.BuildFirst()
      self.PrintFirst()
      self.BuildFollow()
      self.PrintFollow()

   # Add the given rule to the grammar
   def AddRule(self, lhs, rhs):
      assert str(lhs).lstrip() <> ''
      self.AddNonTerminal(lhs)
      rhs_list = []
      for sym in rhs:
         if sym[0] == '<' and sym[-1] == '>':
               symObj = NonTerminal(sym)
               self.AddNonTerminal(symObj)
         else:
               symObj = Terminal(sym)
               self.AddTerminal(symObj)
         rhs_list.append(symObj)

      self.AddProduction(lhs, rhs_list)


   # Read in a CFG and build an object model for it
   def ReadGrammar(self, file):
      fin = open(file, "r") 
      lines = fin.readlines()
      ch = None 
      state = States.NEWPRODUCTION
      lhs = None
      rhs = None

      if (os.path.exists("term.idx") and os.path.isfile("term.idx")):
         idxfile = open("term.idx", "r")
         indices = idxfile.readlines()
         for term in indices:
            self.AddTerminal(Terminal(term.strip()))

      # Assume each rule takes up a line
      # Alternatives for a lhs start on a following line and 
      # begin with "->"
      # Symbols are seperated by at least one space
      # NonTerminals are surrounded by "< >" and no whitespaces in name
      for l in lines: 
         if l.lstrip() == "":
            continue

         first_nt = l.find('<')
         alt = l.find('|')
         if ((alt > -1 and alt < first_nt) or first_nt == -1) and lhs <> None:
            state = States.ALTERNATIVE
         else:
            state = States.NEWPRODUCTION

         # switch
         if state == States.NEWPRODUCTION:
            rule = l.split()
            lhs = NonTerminal(rule[0])
            assert rule[0].lstrip() <> ''
            if len(rule) > 1:
                rhs = rule[1:]
            else:
                rhs = []

            self.AddRule(lhs, rhs)
         elif state == States.ALTERNATIVE:
            rhs = l[l.find("|", 1) + 1 : ].split()
            self.AddRule(lhs, rhs)
  
   def __init__ (self):
      self.productions = {}
      self.nonterminals = []
      self.terminals = []
      self.start = None
      # Maps from NonTerminals to different properties 
      self.nullable = {}
      self.first = {}
      self.follow = {} 
      self.bNullableBuilt = False
      self.bFirstBuilt = False
      self.bFollowBuilt = False
      self.accept = []
   
   # Add rule to dict of productions
   def AddProduction(self, nt, rhs):
      assert isinstance(nt, NonTerminal)
      global rule_id,  rule_num
      # QUESTION: Is using id() a good way to hash??
      rule_id[nt, id(rhs)] = rule_num
      rule_num += 1
      if not self.productions.has_key(nt):
         self.productions[nt] = [rhs]
      else:
         self.productions[nt].append(rhs)
   
   # Add unique nonterminal to list of nonterminals
   def AddNonTerminal(self, nt):
      assert isinstance(nt, NonTerminal)
      # First nonterminal added is assumed to be the start symbol
      if self.start == None:
         self.start = nt
      if nt not in self.nonterminals:
         self.nonterminals.append(nt) 
         self.nullable[nt] = False
         self.first[nt] = []
         self.follow[nt] = []
         global nonterm_id, nonterm_num
         nonterm_id[nt] = nonterm_num
         nonterm_num += 1

   # Add unique terminal to list of terminals
   def AddTerminal(self, t):
      assert isinstance(t, Terminal)
      if t not in self.terminals:
         self.terminals.append(t)
         global term_id, term_num
         term_id[t] = term_num
         term_num += 1

   def PrintNonTerminals(self):
      print "Nonterminals:"
      print "============="
      for nt in self.nonterminals:
         print "\t" + str(nt)

   def PrintTerminals(self):
      print "Terminals:"
      print "============="
      for term in self.terminals:
         print "\t" + (term) 

   def PrintProductions(self):
      print "Productions:"
      print "=============="

      for nt in self.nonterminals:
         print "For ", nt, ":"
         num = 1
         if self.productions.has_key(nt):
            for rule in self.productions[nt]:
               print "\t", num, ". ", nt, str(rule)
               num += 1

   def BuildNullable(self):
      if self.bNullableBuilt:
        return

      bChanged = True 
      while bChanged:
         bChanged = False
         for nt in self.nonterminals:
            if not self.nullable[nt]:
               if self.productions.has_key(nt):
                  for rule in self.productions[nt]:
                     if self.CheckNullable(rule):
                        bChanged = True
                        self.nullable[nt] = True 
               else:
                  print "Warning: No Productions for ", nt

      self.bNullableBuilt = True

   def CheckNullable(self, rhs):
      for sym in rhs:
         if isinstance(sym, Terminal):
            return False
         elif isinstance(sym, NonTerminal):
            if not self.nullable[sym]:
               return False
      return True

   def PrintNullable(self):
      # TODO: optimize
      print "Nullable Table:"
      print "==============="
      for nt in self.nonterminals:
         print str(nt) , " : " , self.nullable[nt]

   def BuildFirst(self):
      if self.bFirstBuilt:
        return

      if not self.bNullableBuilt:
         print "Error: Cannot build First set without Nullable table!"
         return 

      bChanged = True
      while bChanged:
         bChanged = False
         for nt in self.nonterminals:
            if self.productions.has_key(nt):
               for rule in self.productions[nt]:
                  if self.CalculateFirstSet(nt, rule):
                     bChanged = True
            else:
               print "Warning: No Productions for ", nt
      
      self.bFirstBuilt = True

   def CalculateFirstSet(self, nt, rhs):
      bChanged = False
      for sym in rhs:
         if isinstance(sym, Terminal):
            # Add terminal to First of nt
            if sym not in self.first[nt]:
               self.first[nt].append(sym)
               return True
            else:
               return False
         elif isinstance(sym, NonTerminal):
            # Union symbols from First of current sym 
            for x in self.first[sym]:
               if x not in self.first[nt]:
                  self.first[nt].append(x)
                  bChanged = True
            # Stop if current sym is not nullable    
            if not self.nullable[sym]:
               return bChanged
         else:
            assert False

      return bChanged

   def PrintFirst(self):
      if not self.bFirstBuilt:
         print "Error: Must build First table before printing!"
         return
      print
      print "First Table:"
      print "================"
      for nt in self.nonterminals:
         print nt, self.first[nt]

   def BuildFollow(self):
      if self.bFollowBuilt:
        return

      if not self.bNullableBuilt:
         print "Error: Cannot build Follow table without Nullable table!"
         return
      
      if not self.bFirstBuilt:
         print "Error: Cannot build Follow table without First table!"
         return

      fChanged = True

      while fChanged:
         fChanged = False
         for nt in self.nonterminals:
            if self.productions.has_key(nt):
               for rule in self.productions[nt]:
                  if self.CalculateFollowSet(nt, rule):
                     fChanged = True
            else:
               print "Warning: No Productions for ", nt
      
      self.bFollowBuilt = True


   # Calculate Follow Set of nt and NonTerminals in rhs
   def CalculateFollowSet(self, nt, rhs):
      fChanged = False
      idxNode = 0

      for idxNode in range(len(rhs)):
         sym = rhs[idxNode]

         # Only care about symbols following NonTerminals
         if isinstance(sym, NonTerminal):
            i = 0
            # Check out the suffix symbols after sym
            for i in range(idxNode + 1, len(rhs)):
               fSym = rhs[i]

               if isinstance( fSym, Terminal):
                  # If next symbol is a terminal add it to follow of sym
                  if fSym not in self.follow[sym]:
                     self.follow[sym].append(fSym)
                     fChanged = True
                  else:
                     fChanged = False
                  break
               elif isinstance( fSym, NonTerminal):
                  # NonTerminal follows sym (NonTerminal)
                  for x in self.first[fSym]:
                     if x not in self.follow[sym]:
                        self.follow[sym].append(x)
                        fChanged = True
                  
                  if not self.nullable[fSym]:
                     break
               else:
                  assert False
            else:
                # If reached end of rule set-union
                # follow[sym] with follow[nt]
                   for x in self.follow[nt]:
                      if x not in self.follow[sym]:
                         self.follow[sym].append(x)
                         fChanged = True
         idxNode += 1   

      return fChanged
      
   def PrintFollow(self):
      if not self.bFollowBuilt:
         print "Error: Must build Follow table before printing!"
         return
      print
      print "Follow Table:"
      print "================"
      for nt in self.nonterminals:
         print nt, self.follow[nt]


#==============================================================================
class VocabSym:

   def __init__ (self, name):
      self.name = name
#      print "Construct VocabSym"

   def __str__(self):
      return self.name
   
   def __repr__(self):
      return self.__str__()
      
     
   def __eq__ (self, other):
      if other == None:
         return False
      return self.name == other.name

   def __hash__(self):
     return self.name.__hash__()
 
#==============================================================================
class Terminal(VocabSym):

   def __init__ (self, name):
      self.name = name
 #     print "Construct Terminal"

class NonTerminal(VocabSym):

   def __init__ (self, name):
      self.name = name
#      print "Construct NonTerminal"

#==============================================================================
class SententialForm:

   def __init__ (self):
      self.symbols = []
      print "Construct SententialForm"
      
   def AddSymbol(self, symbol):
      self.symbols.append(symbol)

   def __str__(self):
      s = ""
      for symbol in self.symbols:
         s = str(symbol)
      return s

#==============================================================================
# An Item is a production rule along with some notion of progress
# during a parse
class Item:
   
   def __init__(self, lhs, rhs, markerPos):
      global itemNum
      self.lhs = lhs
      self.rhs = rhs
      self.markerPos = markerPos
      self.num = itemNum
      itemNum += 1

   def IsReducible (self):
      return self.markerPos >= len(self.rhs)

   # Return the next symbol after the marker or 
   # the symbol at advance symbols ahead of marker
   def NextSymbol (self, advance = 0):
      if advance + self.markerPos >= len(self.rhs):
         return None
      else:
         return self.rhs[advance + self.markerPos]

   def __repr__(self):
      return self.__str__()

   def __str__(self):
      s = str(self.lhs) + "->"
      m = min(self.markerPos, len(self.rhs))
      for i in range(0, m):
         s += str(self.rhs[i])
      s += "."
      for i in range(m,  len(self.rhs)):
         s += str(self.rhs[i])
      
      return s
  
   def __ne__(self, other):
      return not self.__eq__(other) 

   def __eq__(self, other):
      if other == None:
         return False

      if other.lhs.name <> self.lhs.name:
         return False

      if other.markerPos <> self.markerPos:
         return False

      if other.__str__() <> self.__str__():
         return False

      return True
   

#==============================================================================
# An ItemSet is a set of items that represents a unique parser state.
class ItemSet:
   
   def __init__ (self, items = [], num_kern_items = 1):
      self.items = items
      self.num = None
      self.num_kernel_items = num_kern_items

   def AddItem(self, item):
      self.items.append(item)
   
   def __len__(self):
      return len(self.items)

   def __repr__(self):
      return self.__str__()

   def __str__(self):
      global rule_id
      s = "\nState "+str(self.num)+":\n"
      for itm in self.items:
         s += str(rule_id[itm.lhs, id(itm.rhs)]) + ". " + str(itm) + "\n"
      return s

   def __eq__(self, other):
      if other == None:
         return False

      if len(self.items) <> len(other.items):
         return False

      for i in range(len(self.items)):
         if self.items[i] <> other.items[i]:
            return False
      return True
#==============================================================================
# A LR(0) Parse Table take a LR(1) grammar and outputs definitions in a 
# C++ header file suitable for a LR Parser
class ParseTable:

   # Does state exist for given list of kernelItems?
   def ItemSetExistsFor(self, kernelItems):
        for itmSet in self.states:
            if itmSet.num_kernel_items == len(kernelItems):
                for i in range(itmSet.num_kernel_items):
                    if itmSet.items[i] <> kernelItems[i]:
                        break
                else:
                    return itmSet.num 
        return None

   def GetItemSet(self, kernelItems):
        index = self.ItemSetExistsFor(kernelItems)
        if index <> None:
            return self.states[index]
        return ItemSet(kernelItems, len(kernelItems))

   def __init__(self, grammar):
      self.grammar = grammar
      self.grammar.BuildNullable()
      self.grammar.BuildFirst()
      self.grammar.BuildFollow()
      self.table = {}
      self.states = []
      self.worklist = []
   
   def AddTableRow(self):
      global term_id, nonterm_id, state_num
      nCols = len(term_id) + len(nonterm_id)
      for i in range(nCols):
         self.table[state_num, i] = [0,0,0,0,0]

   # Computes the initial un-closed item set for 
   # a given symbol and position (default is 0)
   def ComputeItemSetFor(self, sym, pos = 0):
      gram = self.grammar
      rules = gram.productions[sym]

      if len(rules) == 0:
         print "Warning: No rules for ", sym
         return

      # Check if state with same kernel already exists
      itm = Item(sym, rules[0], pos)
      itemSet = self.GetItemSet([itm])

      for i in range(1, len(rules)):
         itm = Item(sym, rules[i], pos)
         itemSet.items.append(itm)
      return itemSet

   def ComputeLR0(self):
      global state_num
      startItems = self.ComputeItemSetFor(self.grammar.start)
      self.AddTableRow()
      # Assign state id to initial state
      startItems.num = state_num
      state_num += 1

      self.AddState(startItems)
      while len(self.worklist) > 0:
         s = self.worklist.pop(0)
         self.ComputeGoto(s)

   def AddState(self, items):
      if items not in self.states:
         #print "Before:", self.states
         self.states.append(items)
         self.worklist.append(items)
         #print "After :", self.states

   # Try to advance dot on given sym for items in itemSet
   # return list of relevant items
   def AdvanceDot(self, itemSet, sym):
        rel_items = []
        for item in itemSet.items:
            if item.markerPos < len(item.rhs) and item.rhs[item.markerPos] == sym:
                rel_items.append(Item(item.lhs, item.rhs, item.markerPos + 1))
        return rel_items 

   # Compute the set of closure items for a given item set
   # Closure items are items needed when another item in the set
   # has the marker before a nonteminal
   def Closure(self, itemSet):
      # print "Do closure of ", itemSet
      fChanged = True
      while fChanged:
         fChanged = False
         # For each item
         for itm in itemSet.items:
            # Look at the symbol after the marker
            sym = itm.NextSymbol()
            if isinstance(sym, NonTerminal):
               # Add a fresh item for each rule for sym
                 fresh_items = self.ComputeItemSetFor(sym) 
                # Set Union
                 for x in fresh_items.items:
                     if x not in itemSet.items:
                        itemSet.AddItem(x)
                        fChanged = True 


   def ComputeGoto(self, itemSet):
        global nonterm_id, term_id, rule_id, state_num

        #print "Compute Goto for ", itemSet
        self.Closure(itemSet)

        # Compute reduce actions for itemSet
        fHasReduce = False
        for itm in itemSet.items:
            markerPos = itm.markerPos
            nextSym = itm.NextSymbol()

            # We see an item that is reducible or empty?
            if nextSym == None and itm.IsReducible:
                # If lhs is start symbol then this is an accepting state
                if itm.lhs == self.grammar.start:
                    if itemSet.num not in self.grammar.accept:
                        self.grammar.accept.append(itemSet.num)
#FIXME:                if fHasReduce:
#                    print "Error: Reduce/Reduce Error Detected!"
#                    print str(itm.lhs) + "->" + str(itm.rhs)
#                    sys.exit(-1) 
                fHasReduce = True

                # For each terminal in follow set of lhs mark entry as reduce
                for followTerm in self.grammar.follow[itm.lhs]:
                    self.table[itemSet.num, term_id[followTerm]] =  \
                        [0, 1, 0, nonterm_id[itm.lhs]  + len(term_id), len(itm.rhs)]

        # Compute shifts actions and new states reachable from itemSet
        for symbolSet in [self.grammar.terminals, self.grammar.nonterminals]:
            for sym in symbolSet:
                relevant_items = self.AdvanceDot(itemSet, sym)
                if relevant_items <> []:
                    s_num = self.ItemSetExistsFor(relevant_items)
                    # New state needs to be created
                    if s_num == None:
                        itst = self.GetItemSet(relevant_items)
                        # Assign the new state a state id
                        self.AddTableRow()
                        itst.num = state_num
                        state_num += 1
                        s_num = itst.num
                        self.AddState(itst)
                   
                    # Update table with shift entry
                    tbl_entry = [1, 0, s_num, 0, 0]

                    # Calculate correct symbol id
                    if isinstance(sym, NonTerminal):
                        symId = nonterm_id[sym] + len(term_id)
                    else:
                        symId = term_id[sym]

                    # Detect Shift/Shift conflicts before setting Shift action
                    oldEntry = self.table[itemSet.num, symId]
    # FIXME
    #             if oldEntry[0] == 1:
    #                print "Error: Shift/Shift Conflict detected!"
    #                sys.exit(1)
                    
                    self.table[itemSet.num, symId] = tbl_entry


   # Utility function to format lists as C-style arrays
   def FormatListAsArray(self, list):
      n = len(list)
      s = "{"

      for i in range(n):
         if i < n -1:
            s += str(list[i]) + ","
         else:
            s += str(list[i]) 
      s +=  "}"
      return s

   # Outputs the ParseTable to a file
   def WriteHeaderFile(self, outfile):
        global nonterm_id, term_id, state_num
        num_terms = len(term_id) 
        nCols = len(term_id) + len(nonterm_id)

        fout = open(outfile, "w")

        fout.write("#ifndef __PARSERTABLE_H__\n#define __PARSERTABLE_H__")

        fout.write("\n\n// Number of states,  tokens, non-terminals.\n")
        fout.write("\n#define NUM_STATES " + str(len(self.states)))
        fout.write("\n#define NUM_TOKENS " + str(len(self.grammar.terminals)))
        fout.write("\n#define NUM_NON_TERMINALS " + str(len(self.grammar.nonterminals)) + "\n")

        fout.write("\n\n// Start.")
        fout.write("\n#define STARTSTATE 0")

        fout.write("\n\n// I think we only need one accepting.")
        #fout.write("\n#define ACCEPTSTATE " + str(nonterm_id[self.grammar.start] + num_terms))
		#Usually 2 anyways. Assume there is only one accept state for now
        fout.write("\n#define ACCEPTSTATE " + str(self.grammar.accept[0])) 

        fout.write("\n\n\n// Terminals ")
        for t in term_id.items():
            fout.write("\n//#define " + str(t[0]) + " " + str(t[1]) )
        fout.write("\n\n// NonTerminals ")

        for nt in nonterm_id.items():
            fout.write("\n//#define " + str(nt[0]) + " " + str(nt[1] + num_terms) )

        # Output names of nonterminals in order sorted by index value
        fout.write("\nchar * NON_TERM_NAMES[] = {\n")
        items = nonterm_id.items()
        items.sort(key = itemgetter(1))
        for i in range(len(items)):
            fout.write("\"" + str(items[i][0]) +"\"")
            if i < len(items) -1:
                fout.write(",")
        fout.write("\n};\n")
        fout.write("\n\n")

        blob = """
#define FIRST_NON_TERMINAL_INDEX (NUM_TOKENS)

// Entry of the tabble.
        struct ParserTableEntry {
	// To shift?
	bool shift;

	// To reduce?
	bool reduce;

	// Next state to go if we were to shift.
	unsigned int shiftNextState;

	// The NonTerminal that the handle is being reduced to.
	unsigned int reduceNonTerminal;

	// Number of element in the RHS of the rule.
	unsigned int rhsLen;
};
"""
        fout.write(blob)

        fout.write("\n/**" + str(self.states) +"\n**/")
        fout.write("\n// The big long table.")
        fout.write("\nconst ParserTableEntry ")
        fout.write("\nPARSER_TABLE [NUM_STATES][NUM_TOKENS + NUM_NON_TERMINALS] = \n{\n")

        # Output the transition table 
        for r in range(state_num):
            fout.write("/* State #: " + str(r) + " */ {")
            for c in range(nCols):
                fout.write("/* (" + str(r) + "," + str(c) + ") */")
                fout.write(self.FormatListAsArray(self.table[r, c]))
                if c < nCols-1:
                   fout.write(",")

            if r < state_num -1:
                fout.write("},\n")
            else:
                fout.write("}\n")

        fout.write("};\n\n")

        fout.write("\n// Follow set bitmap.")
        fout.write("\n// Example: set FOLLOW_SET[A][c] = 1, if 'c' is in follow set of A.") 
        fout.write("\n// Note: since nonterminal id's are assumed to continue from terminal id's")
        fout.write("\n// you need to subtract num terminal id from a nonterminal id to index array")
        fout.write("\nconst bool FOLLOW_SET[NUM_NON_TERMINALS][NUM_TOKENS] = ")
        fout.write("\n{\n")

        # Output the follow/lookahead table
        nNonTerms = len(nonterm_id)
        nTerms = len(term_id)
        for r in range(nNonTerms):
            fout.write("{")
            nt = self.grammar.nonterminals[r]
            follow = self.grammar.follow[nt]

            for c in range(nTerms):
                t = self.grammar.terminals[c]
                fout.write(str(int(t in follow)))
                if c < nTerms - 1:
                    fout.write(", ")

            if r < nNonTerms - 1:
                fout.write("},\n")
            else:
                fout.write("}\n")
        fout.write("};\n\n")

        fout.write("\n#endif\n")
        fout.close() 
#============================================================================== 
class States:
   
   NEWPRODUCTION = 1
   ALTERNATIVE = 2
   NONTERMINAL = 3
   TERMINAL = 4
   ARROW = 5
   RHS = 6
   LHS = 7

#==============================================================================
def main():
      if len(sys.argv) < 3:
         print "Usage: cfg_parser <infile> <outfile>"
         return

      # Construct a Grammar and ParseTable to write the header file
      grammar = Grammar()
      grammar.ReadGrammar(sys.argv[1])
      grammar.SelfTest()  
      parseTbl = ParseTable(grammar)
      parseTbl.ComputeLR0()
      print "Accept State(s): ", parseTbl.grammar.accept
      #print parseTbl.states
      parseTbl.WriteHeaderFile(sys.argv[2])
if __name__ == "__main__":
   main()
