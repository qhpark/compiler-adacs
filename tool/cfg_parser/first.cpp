#include "grammar.hpp"
#include "sententialform.hpp"
#include <iostream>

using namespace std;

void Grammar::buildFirst() {
	if (!builtNullable) {
		cout << "Error: Cannot build first table with out nullable table!" << endl;
	}
	
	bool changed = false;
	do {
		changed = false;
		for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
	   	  nt != nonterminals.end(); nt++) {

			RuleNode * curRule = productions[*nt];
			while (curRule) {
				if (calculateFirstSet(*nt, curRule->rule)) {
					changed = true;
				}
				curRule = curRule->next;
			}
		}

	} while (changed);

	builtFirst = true;
}

bool Grammar::calculateFirstSet(NonTerminal nt, SententialForm * rule) {
	
	SententialForm::Node * node = rule->begin();
	bool changed = false;

	while(node) {
		VocabSymbol * sym = node->symbol;

		// Test to see what terminal it is.
		Terminal * term = dynamic_cast<Terminal *>(sym);
		NonTerminal * nonterm = dynamic_cast<NonTerminal *>(sym);

		if (term) {

			if (first[nt].find(*term) == first[nt].end()) {
				first[nt].insert(*term);
				return true;
			} else {
				return false;
			}

		} else if (nonterm) {

			// FIXME: There are some really ineffecient operations here.
			// Since this it a one time thing, I'll leave it for now... -Alan
			unsigned int oldSize = first[nt].size();
			TermSet newSet;
			insert_iterator<TermSet> newSetIterator(newSet, newSet.begin());
			set_union(first[nt].begin(), first[nt].end(),
			          first[*nonterm].begin(), first[*nonterm].end(),
			          newSetIterator);
			

			if (oldSize != newSet.size()) {
				changed = true;
				first[nt] = newSet;
			}
			
			if (!nullable[*nonterm]) {
				return changed;
			}
 
		} else {
			cout << "Found a symbol that is neither terminal nor non-terminal!" << endl;
		}

		// Check the next one.
		node = node->next;
	}
	return changed;
}


void Grammar::printFirst() {

	if (!builtFirst) {
		cout << "Error: Must build First table before printing!" << endl;
		return;
	}

	cout << endl;
	cout << "First Table:" << endl;
	cout << "---------------" << endl;
	for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
	     nt != nonterminals.end(); nt++) {
		cout << *nt << ":\t\t";
		printTerminalSet(first[*nt]);
		cout << endl;
	}
	cout << endl;

}
