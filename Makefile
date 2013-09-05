SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
LDFLAGS = 
CPPFLAGS = 
CXXFLAGS = $(CPPFLAGS) -g
CXX = g++
AS = gas
MAIN = compiler
LEXCS = tool/lexcs/lexcs
DFA_HPP = scannerdfa.hpp
DFASPEC = tool/lexcs/dfa.spec


all: $(DFA_HPP) $(MAIN)

depend: $(DEPENDS)

clean:
	rm -f *.o *.d $(MAIN)
	cd testing && ./clean.sh
	cd tool/cfg_parser && make clean && cd ../../
	cd tool/lexcs && make clean && cd ../../

$(MAIN): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

%.d: %.cpp
	set -e; $(CC) -M $(CPPFLAGS) $< \
          | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
          [ -s $@ ] || rm -f $@

# TOOL
$(LEXCS) : 
	make -C tool/lexcs
$(DFA_HPP) : $(LEXCS) $(DFASPEC)
	$(LEXCS) < $(DFASPEC) > $@		
#include $(DEPENDS)
