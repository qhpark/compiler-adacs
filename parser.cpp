#include "compunit.hpp"
#include "debug.hpp"
#include "parsertable.hpp"
#include "parsetree.hpp"

#include <stack>


using namespace std;

bool CompileUnit::parse() {

	ASSERT(state == SCANNED, "Can only parse after scanning.");
	bool debug = debugOpt.inDebug("parser");

	// -------------------
	// Code for parsing.
	// -------------------
	
	// States
	stack<ParserState> stateStack;
	stateStack.push(STARTSTATE);

	// Root of the tree node.
	stack<ParseTreeNode *> tNodeStack;

	// Interates the whole input stream.	
	list<ParseTreeNode *> inputstream;

	for (std::list<Token *>::const_iterator i = tkStream->begin();
	     i != tkStream->end(); i++) {
		ParseTreeNode * node = new ParseTreeNode((*i)->getType());
		node->setToken(*i);
		inputstream.push_back(node);
	}

	// Place the start of file in front.
	//    Apparently Kwan expect to see SOF if <start> -> SOF <A> EOF
	//    So no need to force SOF to be on the stack.
	//tNodeStack.push(inputstream.front());
	//inputstream.pop_front();


	// Debug message.
	if (debug) {
		cout << endl;
		cout << "Starting to parse " << filename << endl;
		cout << "---------------------- " << endl;
	}


	while (true) {

		// Debug message.
		if (debug) {
			cout << endl << "In State: " << stateStack.top();
			if (stateStack.top() == ACCEPTSTATE) {
				cout << " (accepting)" << endl;
			} else {
				cout << " (non-accepting)" << endl;
			}
		
			// Messages about the state stack.	
			std::stack<ParserState> output = stateStack;
			
			cout << " State Stack: "; 
			while (output.size() > 0) {
				cout << output.top() << " ";
				output.pop();
			}
			cout << "<end>" <<  endl;
		}

		// Stop if we are at accepting.
		if (stateStack.top() == ACCEPTSTATE) {
			break;
		}

		// Current Symbol is used to decide on which state to go next.
		ParseTreeNode * curSymbol = inputstream.front();

		
		if (debug) cout << " Current symbol: " << *curSymbol << endl;
		ParserTableEntry action = 
		 PARSER_TABLE[stateStack.top()][curSymbol->getType()];

		// What action to take?
		bool toShift = action.shift;
		bool toReduce = action.reduce;

		// Shift / Reduce (Must resolve this).
		if (toShift && toReduce) {
		
			if (debug) cout << "Encounted shift / reduce decision" << endl;

			// Peak another token.
			//unsigned int peekTokenType = (inputstream.front())->getType();
			unsigned int peekTokenType = curSymbol->getType();

			// Find the index.
			unsigned int followIdx = action.reduceNonTerminal -
			                           FIRST_NON_TERMINAL_INDEX; 

			// Decide base on the set.
			if (FOLLOW_SET[followIdx][peekTokenType]) {
				if (debug) cout << "Went with reduce." << endl;
				toShift = false;
			} else {
				if (debug) cout << "Went with shift." << endl;
				toReduce = false;
			}
		}

		// To shift or not to shift.
		if (toShift) {
			if (debug) cout << "  Performaning a shift." << endl;

			// Remember where we were.
			stateStack.push(action.shiftNextState);

			// Nothing too intesting about this.
			inputstream.pop_front();
			tNodeStack.push(curSymbol);


		// To reduce or not to reduce.
		} else if (toReduce) {
			if (debug) {
				cout << "  Performaning a reduce:";
				cout << " RHS_type=" << action.reduceNonTerminal;
				cout << " LHS_len=" << action.rhsLen;
				cout << " tNodeStack.size = " << tNodeStack.size();
				cout << endl;
			}

			ParseTreeNode * reduction = 
			    new ParseTreeNode(action.reduceNonTerminal);

			unsigned int nameIdx = action.reduceNonTerminal -
			                           FIRST_NON_TERMINAL_INDEX; 

			reduction->setName(NON_TERM_NAMES[nameIdx]);

			for (int i = 0; i < action.rhsLen; i++) {

				reduction->addChildFront(tNodeStack.top());
				tNodeStack.pop();
				stateStack.pop();

			}

			inputstream.push_front(reduction);

		} else {
	
	
			stack<ParserState> bestStateStack = stateStack;
			list<ParseTreeNode *> bestInputstream = inputstream;
			

			// Find the furthest processed token.
			while (!curSymbol->hasToken())
				curSymbol = curSymbol->getLastChild();

			Token * furthestTk = curSymbol->getToken();

			cout << "Unexpected token: " << *furthestTk << endl;

			if (debugOpt.inDebug("norecovery")) {
				cout << "Did NOT attempt to recover.... " << endl << endl;
				return false;
			}

			cout << "Attempting to recover.... " << endl << endl;

			// Keep recoving to look for the furthest token.
			// Untill it is accepted or it is EOF.
			while (furthestTk && furthestTk->getType() != Token::EOF_) {

				bool gotFurther = false;

				for (int rmPrefix = 0; rmPrefix <=2 && furthestTk; rmPrefix++) {
					for (int rmSuffix = 0; rmSuffix <=2 && furthestTk; rmSuffix++) {
						stack<ParserState> recoverStateStack = stateStack;
						list<ParseTreeNode *> recoverInputstream = inputstream;
						

						Token * resultTk = 
						  recover(recoverStateStack, recoverInputstream,
						          rmPrefix, rmSuffix, debug);

						// Check if we already reached the end.
						if (resultTk == 0) {
							return false;
			
						// Everything else.	
						} else if (furthestTk->comesBefore(*resultTk)) {
							furthestTk = resultTk;
							bestStateStack = recoverStateStack;
							bestInputstream = recoverInputstream;	
							gotFurther = true;
						}
					}
				}
				
				stateStack = bestStateStack;
				inputstream = bestInputstream;

				if (gotFurther) {
					cout << "Unexpected token: " << *furthestTk << endl;
				} else {
					return false;
				}

			}
			return false;
		}
	}

	if (debug)
		cout << endl << "Finished: tNodeStack.size() = " << tNodeStack.size() << endl;

	// Make sure there is only one thing left.
	//ASSERT(tNodeStack.size() == 2, "There can only be 2 elements left in parse tree stack!");
	// Indeed, first rule should be <start> -> BOF <A> EOF


	ASSERT(tNodeStack.size() == 3, "There can only be 3 elements left in parse tree stack!");
	// Get the EOF off the stack.
	tNodeStack.pop();

	// This is what we need.
	parseTreeRoot = tNodeStack.top();

	// All done.
	state = PARSED;

	// Done need this anymore.
	delete tkStream;

	return true;
}


// 
Token * CompileUnit::recover(
               stack<ParserState> & stateStack, 
               list<ParseTreeNode *> & inputstream,
               unsigned int removePrefix,
               unsigned int removeSuffix,
               bool debug) {

	if (debug) {
		cout << endl << "REOVERY-------------------" << endl;
		cout << "prefix= " << removePrefix << " suffix= " << removeSuffix << endl;
	}

	for (int i = 0; i < removePrefix; i++) {
		if (stateStack.size() == 1)
			break;

		stateStack.pop();
	}

	for (int i = 0; i < removeSuffix; i++) {
		if (inputstream.size() == 1)
			break;

		inputstream.pop_front();
	}

	while (true) {

		// Debug message.
		if (debug) {
			cout << "(Recovery) In State: " << stateStack.top();
			if (stateStack.top() == ACCEPTSTATE) {
				cout << " (accepting)" << endl;
			} else {
				cout << " (non-accepting)" << endl;
			}
		}

		// Stop if we are at accepting.
		if (stateStack.top() == ACCEPTSTATE) {
			return 0;
		}

		// Current Symbol is used to decide on which state to go next.
		ParseTreeNode * curSymbol = inputstream.front();

		
		if (debug) cout << "(Recovery) Current symbol: " << *curSymbol << endl;
		ParserTableEntry action = 
		 PARSER_TABLE[stateStack.top()][curSymbol->getType()];

		// What action to take?
		bool toShift = action.shift;
		bool toReduce = action.reduce;

		// Shift / Reduce (Must resolve this).
		if (toShift && toReduce) {
		
			if (debug) cout << "(Recovery) Encounted shift / reduce decision" << endl;

			// Peak another token.
			//unsigned int peekTokenType = (inputstream.front())->getType();
			unsigned int peekTokenType = curSymbol->getType();

			// Find the index.
			unsigned int followIdx = action.reduceNonTerminal -
			                           FIRST_NON_TERMINAL_INDEX; 

			// Decide base on the set.
			if (FOLLOW_SET[followIdx][peekTokenType]) {
				if (debug) cout << "(Recovery) Went with reduce." << endl;
				toShift = false;
			} else {
				if (debug) cout << "(Recovery) Went with shift." << endl;
				toReduce = false;
			}
		}

		// To shift or not to shift.
		if (toShift) {
			if (debug) cout << "(Recovery) Performaning a shift." << endl;

			// Remember where we were.
			stateStack.push(action.shiftNextState);

			// Nothing too intesting about this.
			inputstream.pop_front();


		// To reduce or not to reduce.
		} else if (toReduce) {
			if (debug) cout << "(Recovery) Performaning a reduce." << endl;

			ParseTreeNode * reduction = 
			    new ParseTreeNode(action.reduceNonTerminal);

			for (int i = 0; i < action.rhsLen; i++) {

				// Don't really care.
				stateStack.pop();

			}

			inputstream.push_front(reduction);

		} else {

			// Give up.	
			while (!curSymbol->hasToken())
				curSymbol = curSymbol->getLastChild();
			return curSymbol->getToken();
		}
	}
}
