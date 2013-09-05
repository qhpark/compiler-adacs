/* temporary name lookup table : DON'T COMPILE THIS INTO REAL EXECUTABLE! */

struct _tmp_enum_st {
	int token_enum;    
	char *token_str;
};
/* this is NOT automatically generated.. */
#define NUM_TBL (76)
static struct _tmp_enum_st tok_enum_tbl[] = {
{Token::PRAGMA,         "PRAGMA"},
{Token::IS,             "IS"},
{Token::ARRAY,          "ARRAY"},
{Token::OF,             "OF"},
{Token::RANGE,          "RANGE"},
{Token::RECORD,         "RECORD"},
{Token::TYPE,           "TYPE"},
{Token::SUBTYPE,        "SUBTYPE"},
{Token::CONSTANT,       "CONSTANT"},
{Token::DECLARE,        "DECLARE"},
{Token::BEGIN,          "BEGIN"},
{Token::EXCEPTION,      "EXCEPTION"},
{Token::END,            "END"},
{Token::IF,             "IF"},
{Token::ELSIF,          "ELSIF"},
{Token::ELSE,           "ELSE"},
{Token::LOOP,           "LOOP"},
{Token::WHILE,          "WHILE"},
{Token::FOR,            "FOR"},
{Token::REVERSE,        "REVERSE"},
{Token::EXIT,           "EXIT"},
{Token::CASE,           "CASE"},
{Token::WHEN,           "WHEN"},
{Token::PROCEDURE,      "PROCEDURE"},
{Token::FUNCTION,       "FUNCTION"},
{Token::IN,             "IN"},
{Token::OUT,            "OUT"},
{Token::RETURN,         "RETURN"},
{Token::PACKAGE,        "PACKAGE"},
{Token::BODY,           "BODY"},
{Token::PRIVATE,        "PRIVATE"},
{Token::SEPARATE,       "SEPARATE"},
{Token::USE,            "USE"},
{Token::RAISE,          "RAISE"},
{Token::NULL_,          "NULL_"},
{Token::OTHERS,         "OTHERS"},
{Token::IDENT,          "IDENT"},
{Token::INT_LIT,        "INT_LIT"},
{Token::FLOAT_LIT,      "FLOAT_LIT"},
{Token::STRING_LIT,     "STRING_LIT"},
{Token::ADD,            "ADD"},
{Token::SUB,            "SUB"},
{Token::MULT,           "MULT"},
{Token::DIV ,           "DIV "},
{Token::MOD,            "MOD"},
{Token::ABS,            "ABS"},
{Token::NOT,            "NOT"},
{Token::EXP,            "EXP"},
{Token::EQ,             "EQ"},
{Token::SLASH_EQ,       "SLASH_EQ"},
{Token::LT,             "LT"},
{Token::LTE,            "LTE"},
{Token::GT,             "GT"},
{Token::GTE,            "GTE"},
{Token::AND,            "AND"},
{Token::OR,             "OR"},
{Token::THEN,			"THEN"},
{Token::SINGLE_QUOTE,   "SINGLE_QUOTE"},
{Token::RANGE_OP,       "RANGE_OP"},
{Token::COLON,          "COLON"},
{Token::SEMI_COLON,     "SEMI_COLON"},
{Token::COMMA,          "COMMA"},
{Token::DOT,            "DOT"},
{Token::BAR,            "BAR"},
{Token::AMP,            "AMP"},
{Token::LEFT_BRACE,     "LEFT_BRACE"},
{Token::RIGHT_BRACE,    "RIGHT_BRACE"},
{Token::STAR,           "STAR"},
{Token::ASSIGN,         "ASSIGN"},
{Token::NOT_EQ,         "NOT_EQ"},
{Token::POINTER,        "POINTER"},
{Token::EOF_,           "EOF_"},
{Token::WHITESPACE,     "WHITESPACE"},
{Token::NEWLINE,        "NEWLINE"},
{Token::COMMENT,        "COMMENT"},
{Token::ALL,        "ALL"},
};