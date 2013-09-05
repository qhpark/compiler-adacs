#ifndef _TOKEN_HPP_
#define _TOKEN_HPP_

#include <iostream>
#include <string>

// Token
class Token {
	friend std::ostream& operator<<( std::ostream &out, const Token &T);
	public:
		enum Type { 

			// Start and End.
			BOF=0, EOF_=1, PRAGMA=2, SEMI_COLON=3, IDENT=4,
			PACKAGE=5, IS=6, END=7, BODY=8, TYPE=9, 

			PRIVATE=10, SUBTYPE=11,COLON=12, COMMA=13, CONSTANT=14, 
			ACCESS=15, RECORD=16,NULL_=17, CASE=18, WHEN=19,

			POINTER=20, ARRAY=21, OF=22, LEFT_BRACE=23, RIGHT_BRACE=24,
			RANGE=25, LT=26, GT=27, RANGE_OP=28, ASSIGN=29, 

			PROCEDURE=30, FUNCTION=31, STRING_LIT=32, IN=33, OUT=34, 
			EXCEPTION=35, OTHERS=36, BEGIN=37, DECLARE=38, RETURN=39,

			RAISE=40, IF=41, THEN=42, ELSIF=43, ELSE=44,

			LOOP=45, WHILE=46, FOR=47, REVERSE=48, EXIT=49, 
			BAR=50, AND=51, OR=52, NOT=53, ABS=54, 

			MULT=55, NEW=56, INT_LIT=57, FLOAT_LIT=58, EQ=59, 

			SLASH_EQ=60, LTE=61, GTE=62, ADD=63, SUB=64, 
			AMP=65, DIV=66, MOD=67, SEPARATE=68, USE=69, 

			DOT=70, SINGLE_QUOTE=71, STAR=72, NOT_EQ=73, WHITESPACE=74, 
			NEWLINE=75, COMMENT=76, EXP=77, ALL=78
		};

		Token(Type type);
		// another constructor to handle line, col
		Token(Type type, int line, int col);
		~Token();

		void setValue(const std::string &);
		std::string getValue() const;

		Type getType() const;

		// Return true if it is the same token or this comes
		// before the other token in the file.
		bool comesBefore(Token const & other) const;

		unsigned int getLine() const;
		unsigned int getCol() const;

	private:
		Type type;
		unsigned int lineNum;
		unsigned int charNum;
		std::string value;
};

#endif
