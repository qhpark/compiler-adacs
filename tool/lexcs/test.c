#include <stdio.h>
#include <string.h>
type enum {
	IF, BEGIN, RBRACE, INCREMENT, DECREMENT,
	NUMBER, IDENTIFIER	
};
#include "../scannerdfa.hpp"
char *scode = "if 2.41";
int main()
{
	int last_term= 0;	
	int c_pos = 0; 
	int c_state = 1;
	
	char buf[1024];

	while (*scode) {
		// c_pos -> updated if last_term != next_state
		// 
		c_state = DFA[c_state].map[*scode];
		if (c_state == 0) {
			// error, so look at last_term.
			// if last_term is 0, it's an error, report.
			if (last_term == 0) {
				printf("Error!\n");
				scode++;
				continue;
			}
		}
		// check if c_state is terminal
		if (DFA[c_state].terminal)
			last_term = c_state;
			
	}
	return 0;
}
	
