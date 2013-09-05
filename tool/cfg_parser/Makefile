SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
LDFLAGS = 
CPPFLAGS = 
CXXFLAGS = $(CPPFLAGS) -g
CXX = g++
MAIN = cfg_parser 

all: $(MAIN)

depend: $(DEPENDS)

clean:
	rm -f *.o *.d $(MAIN)

$(MAIN): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

%.d: %.cpp
	set -e; $(CC) -M $(CPPFLAGS) $< \
          | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
          [ -s $@ ] || rm -f $@

#include $(DEPENDS)

