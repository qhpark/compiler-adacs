#include "grammar.hpp"
#include "sententialform.hpp"
#include <iostream>

using namespace std;

void Grammar::buildNullable() {

	for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
	     nt != nonterminals.end(); nt++) {
		nullable[*nt] = false;
	}

	bool changed = false;
	do {
		changed = false;
		for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
		     nt != nonterminals.end(); nt++) {

			if (!nullable[*nt]) {
				RuleNode * curRule = productions[*nt];
				while (curRule) {
					if (checkNullable(curRule->rule)) {
						changed = true;
						nullable[*nt] = true;
					}
					curRule = curRule->next;
				}
			}
		}
	} while (changed);

	builtNullable = true;
}


bool Grammar::checkNullable(SententialForm* rule) {
	SententialForm::Node * node = rule->begin();
	while(node) {

		Terminal * term = dynamic_cast<Terminal *>(node->symbol);
		NonTerminal * nonterm = dynamic_cast<NonTerminal *>(node->symbol);
		if (term) {
			return false;
		} else if (nonterm) {
			if (nullable[*nonterm]) {
				node = node->next;
			} else {
				return false;
			}
		}
	}
	return true;
}

void Grammar::printNullable() {

	if (!builtNullable) {
		cout << "Warning nullable table not built!" << endl;
	}

	cout << endl;
	cout << "Nullable Table:" << endl;
	cout << "---------------" << endl;
	for (std::set<NonTerminal>::const_iterator nt = nonterminals.begin();
	     nt != nonterminals.end(); nt++) {
		cout << *nt << "\t\t\t\t";
		if (nullable[*nt]) {
			cout << "nullable";
		} else {
			cout << "--------";
		}
		cout << endl;
	}
	cout << endl;
}
