#include "compunit.hpp"
#include "debug.hpp"
#include "token.hpp"
#include "scannerdfa.hpp"
#include "ringbuffer.hpp"
#include <list>
#include <fstream>

using namespace std;
#define DEBUG_TOKEN_NAME
#ifdef DEBUG_TOKEN_NAME 
// remove this later!!!!!
#include "debugtable.hpp"

static char * __find_type(Token::Type tok) {
	for (int i = 0 ; i < NUM_TBL ; i++) 
		if (tok_enum_tbl[i].token_enum == tok)
			return  tok_enum_tbl[i].token_str;
	return "##ERROR";
}
#endif

// Helper
static Token::Type find_token(const char *tok) {
	for (int i = 0 ; i < num_rsvd_tbl ; i++) 
		if (!strcmp(tok, rsvd_table[i].rsvd))
			return (Token::Type) rsvd_table[i].token;
	return Token::IDENT;
}
static char* to_lowers(const char *tok, char* dst ) {
	char *tmp = dst;
	while (*dst++ = (char) tolower(*tok++));
	return tmp;
}


bool CompileUnit::scan() {
	ASSERT(state == SOURCE, "Compile Unit can only be scanned in source state.");
	tkStream = new std::list<Token *>();

	RingBuffer rb(2048);

	std::ifstream srcFile;
	srcFile.open(filename.c_str(), std::ios::in);

	if (debugOpt.inDebug("scan"))
		std::cout << "reading from file! : " << filename<<std::endl;

	char inChar;
	unsigned int curLineNum = 1;
	unsigned int curCharNum = 1; // is this supposed to be column??
								 // i'm assuming it starts from 0
								 // when it hits the new character
								 
	int cs = 1;  // current state, initially it's 1
	int read = 0;// read pointer in the buffer
	int lfs = 0; // last final state.
	int fp = 0;  // position of the character when it's at the final
	             // state. lsf and fp always go together.
	string token_str;
	Token::Type tok;

	// Beginning of the file.
	tkStream->push_back(new Token(Token::BOF));

	for (;;) {
		// check if there's stuff to read off the buffer
		if (rb.num_stored() && rb.num_unread()) {
			read = rb.get_char(inChar);
		} else if (srcFile.eof()) {
			// nothing to read, quit. quit quit....!!
			break; 
		} else {
			// read, add, get processed..
			srcFile.get(inChar);
			if (!rb.num_free()) {
				cerr << filename << ":" << curLineNum<< 
					" error: Token too long " << endl;
				return false;	
			}
			rb.add_char(inChar);
			read = rb.get_char(inChar);
			ASSERT(read != -1, "BUG!!"); 
		}

		// now we have current state, character..
		// let's walk on some DFA edges..
		
		if (debugOpt.inDebug("scan"))
			cout << "current state: "<< cs << " char: "<< inChar<< endl;
		// update cs
		cs = DFA[cs].map[inChar];
		if (debugOpt.inDebug("scan"))
			cout << "new current state: "<< cs << " char: "<< inChar<< endl;
		
		if (cs) {
			// non-zero state
			if (DFA[cs].terminal) {
				lfs = cs;
				fp = read;
			}
		} else {
			// zero state, either error or we've read some valid
			// token (as far as we can tell. oh well)

			if (lfs) {
				// have some token to return
				ASSERT(fp, "BUG!!");
				// check the token type
				// any token not uniquely identified by DFA must
				// be searched for the correct token type
				// whitespaces are ignored
				
				token_str = rb.flush_str(fp);
				if (debugOpt.inDebug("scan"))
					cout << "TOKEN: "<< token_str << endl;

				// handle special case
				tok = (Token::Type) DFA[lfs].token;
				char dst[fp + 1];	
				switch (tok) {
					Token * token;
					case Token::IDENT:
					// case Token::ANY_OTHER_RESERVED:
						tok = find_token(to_lowers(token_str.c_str(), dst)); 
						token = new Token(tok, curLineNum, curCharNum);
						if (tok == Token::IDENT) {
							token->setValue(dst);
						}
						tkStream->push_back(token);
						break;
					case Token::INT_LIT:
					case Token::FLOAT_LIT:
					case Token::STRING_LIT:
						token = new Token(tok, curLineNum, curCharNum);
						token->setValue(token_str);
						tkStream->push_back(token);
						break;
					case Token::WHITESPACE:
					case Token::NEWLINE:
					case Token::COMMENT:
						// do nothing.
						break;
					default:
						token =
							new Token(tok, curLineNum, curCharNum);
						tkStream->push_back(token);
				}
#ifdef DEBUG_TOKEN_NAME
				if (tok != Token::WHITESPACE && tok != Token::NEWLINE && debugOpt.inDebug("token")) {
					cout << "TOKEN["<< __find_type(tok)<<"] : "<< token_str << 
						  "  line: " << curLineNum
						 << " col: " << curCharNum <<endl;
				}
#endif
				// update line, col numbers
				if (tok == Token::NEWLINE) {
					curLineNum += fp;
					curCharNum = 1;
				} else 
					curCharNum += fp;
			} else {
				// error
				static int last_err_line = 0;
				token_str = rb.flush_str(1);
				if (last_err_line != curLineNum) {
					last_err_line = curLineNum;
					cerr << filename << ":" << curLineNum<< 
						" error: not a valid token - " 
					    << token_str << endl;
				}
			}
			// reset pointers
			cs = 1;
			lfs = 0;
			fp = 0;
		}
		
	}

	// ADD EOF to the stream.
	tkStream->push_back(new Token(Token::EOF_));

	srcFile.close();
	state = SCANNED;
	return true;
	//return false;
}
