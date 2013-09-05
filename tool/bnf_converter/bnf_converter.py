#!/usr/bin/python

# Utility to convert CFGs written using BNF into standard
# notation.
#
# Assumptions:
# Each production rule must only occupy a single line

import sys, string

nextNT = 0

def main():
   if len(sys.argv) == 1:
      print "Usage: bnf_converter <filename>"
      return
   fin = open(sys.argv[1])
   lines = fin.readlines()
   cur_lhs = None

   for l in lines:
      # Find the current Lhs NonTerminal
      if l[0] == '<':
         cur_lhs = l[0 : l.find('>') + 1]

      if '[' in l or '{' in l:
         handleBnf(cur_lhs, l)
      else :
         print l,


# Recursively
def handleBnf(lhs, rule):
   # Build list of new productions without BNF
   lenRule = len(rule)
   for i in range(lenRule):
      if rule[i] == '[':
         startChar = '['
         endChar = ']'
         break
      elif rule[i] == '{':
         startChar = '{'
         endChar = '}'
         break

   bnfSym = None
   cnt = 1
   for j in range(i+1, lenRule):
      if rule[j] == endChar:
         cnt -= 1
         if cnt == 0:
            bnfSym = rule[i : j + 1]
            break
      elif rule[j] == startChar:
         cnt += 1

   # Create new productions using new nonterminal
   global nextNT 
   nextNTName = "<NT" + str(nextNT) + '>'
   rule = rule.replace(bnfSym, nextNTName, 1)
   nextNT += 1
   if rule.find('[') > -1 or rule.find('{') > -1:
      handleBnf(nextNTName, rule)
   else:
      print rule
   if (startChar == '{'):
      print nextNTName + " -> " + bnfSym[1:-1] + " " + nextNTName
   else:
      print nextNTName + " -> " + bnfSym[1:-1]
   print nextNTName + " -> " 

if __name__ == "__main__":
   main()
