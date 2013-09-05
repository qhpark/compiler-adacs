#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include <string>
#include <vector>
#include <iostream>

// Assertion.
#define ASSERT(test,msg) \
    ( (test) ? (void)0 \
    : (std::cerr << __FILE__ "(" << __LINE__ \
                 << "): Assertion failed." << std::endl \
                 << __FILE__ "(" << __LINE__ << "):  " \
                 << msg << std::endl, \
       abort()));
// Debug:
//  Determine if a -D option is used.
class Debug {
	friend std::ostream& operator<<( std::ostream &out, const Debug &D);
	public:
		Debug();
		~Debug();

		// Supply an option.
		void addDebug(std::string);

		// Check if a -D option is used.
		bool inDebug(std::string);


	private:
		bool sorted;
		std::vector<std::string> options;
};

extern Debug debugOpt;
#ifdef REMOVE_DEBUG
#define debugOpt 0 && debugOpt
#endif 
#endif
