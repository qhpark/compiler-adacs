#include "timer.hpp"
#include "debug.hpp"
#include <time.h>
#include <iostream>
#include <vector>

TimerEntry::TimerEntry(std::string aName) : 
  start(clock()), end(0), name(aName), running(true) { 

}

TimerEntry::~TimerEntry() { }

Timer::Timer() : initTime(clock()) { }
Timer::~Timer() { }

TimerEntryIndex Timer::addEntry(std::string name) {
	TimerEntryIndex idx = entries.size();
	entries.push_back(TimerEntry(name));
	entries[idx].start = clock();
	return idx;
}

void Timer::start(TimerEntryIndex idx) {
	ASSERT(idx < entries.size(), "Invalid idx for timer");
	entries[idx].start = clock();
}

void Timer::stop(TimerEntryIndex idx) {
	ASSERT(idx < entries.size(), "Invalid idx for timer");
	entries[idx].end = clock();
	entries[idx].running = false;
}

std::ostream& operator<<( std::ostream &out, const Timer &t) {
	out << "--------------------------------------------------" << std::endl;
	out << "Timing Statistics" << std::endl;
	out << "--------------------------------------------------" << std::endl;

	clock_t now = clock();
	clock_t total = now - t.initTime;
	for (std::vector<TimerEntry>::const_iterator i = t.entries.begin();
		 i != t.entries.end(); i++) {

		clock_t end = (*i).end;
		if ((*i).running) {
			end = now;
		}

		if (end != (*i).start) {
			clock_t subTotal = end - (*i).start;
			double percentage = total != 0.0 ? (subTotal * 100.0) / total  : 0.0;
			printf("  %-30s %f %\n", (*i).name.c_str(), percentage);

		} else {
			printf("  %-30s (insignificant) \n", (*i).name.c_str());
		}
	}

	out << std::endl;
	out << "--------------------------------------------------" << std::endl;
	out << "     Total Clock() = " << now << std::endl;
	out << "--------------------------------------------------" << std::endl;
	return out;
}
