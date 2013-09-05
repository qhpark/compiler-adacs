#include "grammar.hpp"
#include "sententialform.hpp"
#include <iostream>

using namespace std;

void Grammar::buildFollow() {

	if (!builtNullable) {
		cout << "Error: Cannot build follow table with out nullable table!" << endl;
	}
	if (!builtFirst) {
		cout << "Error: Cannot build follow table with out first table!" << endl;
	}
	
	bool changed = false;
	do {
		changed = false;
		for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
	   	  nt != nonterminals.end(); nt++) {

			RuleNode * curRule = productions[*nt];
			while (curRule) {
				if (calculateFollowSet(*nt, curRule->rule)) {
					changed = true;
				}
				curRule = curRule->next;
			}
		}

	} while (changed);

	builtFollow = true;
}

bool Grammar::calculateFollowSet(NonTerminal nt, SententialForm * rule) {
	
	SententialForm::Node * node = rule->begin();
	bool changed = false;

	while(node) {
		VocabSymbol * sym = node->symbol;

		// Test to see what terminal it is.
		NonTerminal * nonterm = dynamic_cast<NonTerminal *>(sym);

		if (nonterm) {

			// FIXME: There are some really ineffecient operations here.
			// Since this it a one time thing, I'll leave it for now... -Alan


			SententialForm::Node * canFollow = node->next;
			while (canFollow) {
				Terminal * fTerm = dynamic_cast<Terminal *>(canFollow->symbol);
				NonTerminal * fNonterm = dynamic_cast<NonTerminal *>(canFollow->symbol);

				// If it is a terminal. That is in the suffix before a non-terminal.
				if (fTerm) {

					// All other suffix vocab will not be in the follow.
					// (At least not from this rule.
					if (follow[*nonterm].find(*fTerm) == follow[*nonterm].end()) {
						follow[*nonterm].insert(*fTerm);
						changed = true;
					} else {
						changed = false;
					}
					break;

				} else if (fNonterm) {

					unsigned int oldSize = follow[*nonterm].size();
					// New follow set for comparison.
					TermSet newSet;
					insert_iterator<TermSet> newSetIterator(newSet, newSet.begin());

					set_union(follow[*nonterm].begin(), follow[*nonterm].end(),
					          first[*fNonterm].begin(), first[*fNonterm].end(),
					          newSetIterator);

					if (oldSize != newSet.size()) {
						changed = true;
						follow[*nonterm] = newSet;
					}

					if (nullable[*fNonterm]) {
						canFollow = canFollow->next;
					} else {
						break;
					}
				}

			}

			// If we ever reach the very end of the production,
			// We need to add the follow-set of the parent to the follow-set
			// of the current non-terminal.
			if (canFollow == 0) {

				// Add the parent set here.
				unsigned int oldSize = follow[*nonterm].size();
				// New follow set for comparison.
				TermSet newSet;
				insert_iterator<TermSet> newSetIterator(newSet, newSet.begin());

				set_union(follow[*nonterm].begin(), follow[*nonterm].end(),
				          follow[nt].begin(), follow[nt].end(),
				          newSetIterator);

				if (oldSize != newSet.size()) {
					changed = true;
					follow[*nonterm] = newSet;
				}

			}
		} 
		  
		// Check the next one.
		node = node->next;
	}

	return changed;
}

void Grammar::printFollow() {

	if (!builtFollow) {
		cout << "Error: Must build Follow table before printing!" << endl;
		return;
	}

	cout << endl;
	cout << "Follow Table:" << endl;
	cout << "---------------" << endl;
	for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
	     nt != nonterminals.end(); nt++) {
		cout << *nt << ":\t\t";
		printTerminalSet(follow[*nt]);
		cout << endl;
	}
	cout << endl;

}
