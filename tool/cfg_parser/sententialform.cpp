#include "sententialform.hpp"
#include <typeinfo>

// Constructor
SententialForm::SententialForm() : numSymbols(0) {
	head = NULL;
	tail = NULL;
}

// Destructor
SententialForm::~SententialForm() {
	Node *node = head;
	while(node != NULL) {
		Node* tmp = node;
		node = node->next;
		delete tmp;
	}
}


SententialForm::Node* SententialForm::begin() const {
	return head;
}

// Append given symbol to end of sentence.
//
// Memory Ownership:
// SententialForm owns sym and is responsible for its deallocation.
void SententialForm::AddSymbol(VocabSymbol* sym) {
	Node *newNode = new Node();
	newNode->next = NULL;
	newNode->symbol = sym;
	if(head == NULL) {
		head = tail = newNode;
	} else {
		tail->next = newNode;
		tail = newNode;
	}
	numSymbols++;
}

// Overload the stream output operator
std::ostream& operator<< (std::ostream &out, const SententialForm &SF) {
	SententialForm::Node *node = SF.head;
	if(node == NULL) {
		out<<" @";
	} else {
		for(; node != NULL; node = node->next ) {
			out<<" "<<*(node->symbol);
		}
	}
	out<<std::endl;
	return out;
};

unsigned int SententialForm::getNumSymbols() const {
	return numSymbols;
}
