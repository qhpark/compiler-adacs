#include <iostream>
#include "token.hpp"
#include "debug.hpp"

Token::Token(Token::Type t) {
	type = t;
}
Token::Token(Token::Type t, int line, int col ) {
	type = t;
	lineNum = line;
	charNum = col;
}

Token::~Token() {
//	if (debugOpt.inDebug("tokens")) {
//		std::cout << "Deleting Token: " << *this << std::endl;
//	}
} 
void Token::setValue(const std::string & val) {
	value = val;
}

std::string Token::getValue() const {
	return value;
}

Token::Type Token::getType() const {
	return type;
}

unsigned int Token::getLine() const {
	return lineNum;
}
unsigned int Token::getCol() const {
	return charNum;
}

bool Token::comesBefore(Token const & other) const {
	if (lineNum == other.lineNum) {
		return charNum < other.charNum;
	} else {
		return lineNum < other.lineNum;
	}
}
std::ostream& operator<<( std::ostream &out, const Token &tk) {
	switch (tk.type) {
		case Token::BOF: out << "[BOF"; break;
		case Token::EOF_: out << "[EOF_"; break;
		case Token::PRAGMA: out << "[KW-PRAGMA"; break;
		case Token::IDENT: out << "[ID name=\"" << tk.getValue() << "\"" ; break;
		case Token::SEMI_COLON: out << "[;"; break;

		case Token::PACKAGE: out << "[KW-PACKAGE"; break;
		case Token::IS: out << "[KW-IS"; break;
		case Token::END: out << "[KW-END"; break;
		case Token::BODY: out << "[KW-BODY"; break;
		case Token::TYPE: out << "[KW-TYPE"; break;

		case Token::PRIVATE: out << "[KW-PRIVATE"; break;
		case Token::SUBTYPE: out << "[KW-SUBTYPE"; break;
		case Token::COLON: out << "[:"; break;
		case Token::COMMA: out << "[,"; break;
		case Token::CONSTANT: out << "[KW-CONSTANT"; break;

		case Token::ACCESS: out << "[KW-ACCESS"; break;
		case Token::RECORD: out << "[KW-RECORD"; break;
		case Token::NULL_: out << "[NULL"; break;
		case Token::CASE: out << "[KW-CASE"; break;
		case Token::WHEN: out << "[KW-WHEN"; break;

		case Token::POINTER: out << "[KW-POINTER"; break;
		case Token::ARRAY: out << "[KW-ARRAY"; break;
		case Token::OF: out << "[KW-OF"; break;
		case Token::LEFT_BRACE: out << "[("; break;
		case Token::RIGHT_BRACE: out << "[)"; break;

		case Token::RANGE: out << "[KW-RANGE"; break;
		case Token::LT: out << "[<"; break;
		case Token::GT: out << "[>"; break;
		case Token::RANGE_OP: out << "[.."; break;
		case Token::ASSIGN: out << "[:="; break;

		case Token::PROCEDURE: out << "[KW-PROCEDURE"; break;
		case Token::FUNCTION: out << "[KW-FUNCTION"; break;
		case Token::STRING_LIT: out << "[STRING LIT \"" << tk.getValue() << "\"" ; break;
		case Token::IN: out << "[KW-IN"; break;
		case Token::OUT: out << "[KW-OUT"; break;

		case Token::EXCEPTION: out << "[KW-EXCEPTION"; break;
		case Token::OTHERS: out << "[KW-OTHERS"; break;
		case Token::BEGIN: out << "[KW-BEGIN"; break;
		case Token::DECLARE: out << "[KW-DECLARE"; break;
		case Token::RETURN: out << "[KW-RETURN"; break;

		case Token::RAISE: out << "[KW-RAISE"; break;
		case Token::IF: out << "[KW-IF"; break;
		case Token::THEN: out << "[KW-THEN"; break;
		case Token::ELSIF: out << "[KW-ELSIF"; break;
		case Token::ELSE: out << "[KW-ELSE"; break;

		case Token::LOOP: out << "[KW-LOOP"; break;
		case Token::WHILE: out << "[KW-WHILE"; break;
		case Token::FOR: out << "[KW-FOR"; break;
		case Token::REVERSE: out << "[KW-REVERSE"; break;
		case Token::EXIT: out << "[KW-EXIT"; break;

		case Token::BAR: out << "[|"; break;
		case Token::AND: out << "[KW-AND"; break;
		case Token::OR: out << "[KW-OR"; break;
		case Token::NOT: out << "[KW-NOT"; break;
		case Token::ABS: out << "[ABS"; break;

		case Token::MULT: out << "[*"; break;
		case Token::NEW: out << "[KW-NEW"; break;
		case Token::INT_LIT: out << "[INT value=" << tk.getValue(); break;
		case Token::FLOAT_LIT: out << "[FLOAT value=" << tk.getValue(); break;
		case Token::EQ: out << "[="; break;

		case Token::SLASH_EQ: out << "[/="; break;
		case Token::GTE: out << "[>="; break;
		case Token::LTE: out << "[<="; break;
		case Token::ADD: out << "[+"; break;
		case Token::SUB: out << "[-"; break;

		case Token::AMP: out << "[&"; break;
		case Token::DIV: out << "[/"; break;
		case Token::MOD: out << "[%"; break;
		case Token::SEPARATE: out << "[KW-SEPARATE"; break;
		case Token::USE: out << "[KW-USE"; break;

		case Token::DOT: out << "[.";  break;
		case Token::SINGLE_QUOTE: out << "[KW-SINGLE_QUOTE"; break;
		case Token::STAR: out << "[STAR?"; break;
		case Token::NOT_EQ: out << "[NOT_EQ"; break;
		case Token::WHITESPACE: out << "[WHITESPACE"; break;


		case Token::NEWLINE: out << "[NEWLINE"; break;
		case Token::COMMENT: out << "[COMMENT"; break;
		case Token::EXP: out << "[EXP"; break;
		case Token::ALL: out << "[ALL"; break;


		// Not finished.
		// REALLY HAVE TO FINISH IT!
		default: out << "[UNKNOWN TOKEN  "; break;


	}

	out << " type=" << tk.type;
	out << " line=" << tk.lineNum;
	out << " char=" << tk.charNum;
	out << "]";
	return out;
}
