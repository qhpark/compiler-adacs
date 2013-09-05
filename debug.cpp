#include "debug.hpp"
#include <algorithm>
#include <iostream>

Debug::Debug() : sorted(true) { }

Debug::~Debug() { }

void Debug::addDebug(std::string opt) {
	options.push_back(opt);
	sorted = false;
}

bool Debug::inDebug(std::string opt) {
	if (!sorted) {
		sort(options.begin(), options.end());
		sorted = true;
	}
	return binary_search(options.begin(), options.end(), opt);
}

std::ostream& operator<<( std::ostream &out, const Debug &d) {
	out << "DebugOpt={";
	for (std::vector<std::string>::const_iterator i = d.options.begin();
		 i != d.options.end(); i++) {
		out << *i << " ";
	}
	out << "}";
	return out;
}
