#ifndef _SENTENTIALFORM_HPP_
#define _SENTENTIALFORM_HPP_

/*
 * Represents a sentence of symbols used on the Rhs of production rules.
 */

#include <iostream>
#include <vector>
#include "vocabsym.hpp"

class SententialForm {
	friend std::ostream& operator<< (std::ostream &out, const SententialForm &SF);

	public:
		
		// Constructor
		SententialForm();
		
		// Destructor
		~SententialForm();
		
		// Append given symbol to end of sentence
		void AddSymbol(VocabSymbol* sym);
		
		// Sentence is a linked-list
		struct Node {
			Node* next;
			VocabSymbol* symbol;
		};

		unsigned int getNumSymbols() const;

		Node* begin() const;


	private:
		Node *head, *tail;
		unsigned int numSymbols;

};

#endif
