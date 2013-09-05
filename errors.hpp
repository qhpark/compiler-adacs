#ifndef _ERRORS_H_
#define _ERRORS_H_

// Error messages for errors during parsing
enum ErrCode {
    UneclaredVar = 0,
    MismatchName = 1,
    IdMismatch = 2,
    NotSupported = 3,
};

char * ErrMsgs[] = 
{ "Variable undeclared before use.",			// 0
  "Mismatched name in package declaration.",	// 1
  "id and optional id don't match.",			// 2
  "Not supported yet.",							// 3
};

#define PRINT_ERRORS(file, tok, type) \
		cerr << file << ":" << tok->getLine() << "." << tok->getCol() \
			 << ": " << *(tok) << " " << ErrMsgs[type] << endl   
#endif
