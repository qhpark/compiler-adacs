# SPEC File
[RESERVED]

# ---------------KEYWORDS------------------
# Pragmas
Token::PRAGMA pragma
# Types
Token::IS is
Token::ARRAY array
Token::OF of
Token::RANGE range
Token::RECORD record
Token::TYPE type
Token::SUBTYPE subtype
Token::ACCESS access
Token::NEW new

# Constants
Token::CONSTANT constant

# Blocks
Token::DECLARE    declare 
Token::BEGIN      begin
Token::EXCEPTION  exception
Token::END        end

# If Statements
Token::IF if
Token::ELSIF elsif
Token::ELSE  else

# Loop Statements
Token::LOOP     loop     
Token::WHILE    while
Token::FOR      for
Token::REVERSE  reverse

# Exit Statement
Token::EXIT exit

# Case Statement
Token::CASE case 
Token::WHEN when

# Subprograms
Token::PROCEDURE  procedure
Token::FUNCTION   function
Token::IN         in
Token::OUT        out
Token::RETURN     return

# Packages
Token::PACKAGE    package
Token::BODY       body
Token::PRIVATE    private
Token::SEPARATE   separate
Token::USE        use

# Exceptions
Token::RAISE raise

# Reserved Keywords
Token::NULL_ null
Token::OTHERS others

# Others
Token::ABS abs
Token::NOT not

# LogicalOps
Token::AND and
Token::OR or
Token::THEN then

# updated
Token::ALL all

# ---------------KEYWORDS ENDS-------------



# (almost) COMPLETE DFA..........

[DFA]
# '-' - from to.
# esc character  \
# \\ for '\'
# \- for '-'
# \0\-\255\ from 0 to 255 

#	STATE	INPUT			NEXT	ENUM
	i.0		[0-9]			S_0 
	S_0.1	[0-9_]			S_0		Token::INT_LIT 
	S_0.1	[.]				S_1_0	Token::INT_LIT 
	S_1_0.0	[0-9]			S_1		
	S_1.1	[0-9_]			S_1		Token::FLOAT_LIT
	S_1.1	[Ee]			S_4		Token::FLOAT_LIT
	S_4.0	[\-+0-9]		S_5		
	S_0.1	[Ee]			S_4		Token::INT_LIT
	S_5.1	[0-9_]			S_5		Token::FLOAT_LIT
                               
# Identifier                    
	i.0		[a-zA-Z]		S_8 
	S_8.1	[A-Za-z0-9_]	S_8		Token::IDENT
	
# whitespace
# CR,LF,SPACE, TAB
	i.0		[\9\\32\\13\]	W		
	i.0		[\10\]			N		
	N.1		[\10\]	        N		Token::NEWLINE
	W.1		[\9\\32\\13\]   W		Token::WHITESPACE

# +
	i.0		[+]				S_9_0	
	S_9_0.1	[]				0		Token::ADD
# -
	i.0		[\-]			S_9_1	
	S_9_1.1	[\-]			C		Token::SUB
# -- COMMENTS
	C.1		[\9\\32\-~] C Token::COMMENT

# * and **
	i.0		[*]				S_9_2	
	S_9_2.1	[*]				S_9_3	Token::MULT
	S_9_3.1	[]				0		Token::EXP

# >= 
	i.0		[>]				S_15	
	S_15.1	[=]				S_18	Token::GT
	S_18.1	[]				0		Token::GTE

# '
	i.0		 [']			S_12_0	
	S_12_0.1 []				0		Token::SINGLE_QUOTE

# ; semicolon' 
	i.0		[;]				S_12_1	
	S_12_1.1 []				0		Token::SEMI_COLON
# | 
	i.0		[|]				S_12_2	
	S_12_2.1 []				0		Token::BAR
# & 
	i.0		[&]				S_12_3	
	S_12_3.1 []				0		Token::AMP
# ( 
	i.0		[(]				S_12_4	
	S_12_4.1 []				0		Token::LEFT_BRACE
# ) 
	i.0		[)]				S_12_5	
	S_12_5.1 []				0		Token::RIGHT_BRACE
# , 
	i.0		[,]				S_12_6	
	S_12_6.1 []				0		Token::COMMA


# => 
	i.0		[=]				S_16	
	S_16.1  [>]				S_17	Token::EQ
	S_17.1  []				0		Token::POINTER
# := 
	i.0		[:]				S_19	
	S_19.1  [=]				S_20	Token::COLON
	S_20.1  []				0		Token::ASSIGN
# .. 
	i.0		[.]				S_21	
	S_21.1  [.]				S_22	Token::DOT
	S_22.1  []				0		Token::RANGE_OP

# /= 
	i.0		[/]				S_10	
	S_10.1  [=]				S_11	Token::DIV
	S_11.1  []				0		Token::SLASH_EQ

# <=, <>
	i.0		[<]				S_13	
	S_13.1  [>]				S_11	Token::LT
	S_13.1  [=]				S_14	Token::LT
	S_14.1  []				0		Token::GTE

# string
	i.0		["]				S_6	
	S_6.0   ["]				S_7		
	S_6.0   [\9\\32\!#-~]	S_6		
	S_7.1	["]				S_6		Token::STRING_LIT
