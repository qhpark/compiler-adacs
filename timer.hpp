#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <vector>
#include <string>
#include <time.h>

// Timer:
typedef unsigned int TimerEntryIndex;

// Entry of a timer. 
// Every timed operation needs an entry with a name here.
class TimerEntry {
	public:
		TimerEntry(std::string aName);
		~TimerEntry();
		clock_t start;
		clock_t end;
		std::string name;
		bool running;
};

// Timer:
// Measure how clock_t an operation took.
class Timer {
	friend std::ostream& operator<<( std::ostream &out, const Timer &D);
	public:
		Timer();
		~Timer();

		// Supply an option.
		TimerEntryIndex addEntry(std::string name);
		void start(TimerEntryIndex idx);
		void stop(TimerEntryIndex idx);

	private:
		clock_t initTime;
		std::vector<TimerEntry> entries;
};
extern Timer timer;

#endif
