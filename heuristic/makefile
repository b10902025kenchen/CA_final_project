CXX = g++
CXXFLAGS = -std=c++17 -O3

SOURCES := $(wildcard [0-1][0-1][0-1].cpp)	#from 000 to 111
OBJECTS := $(SOURCES:.cpp=.o)
TARGETS := $(SOURCES:.cpp=)

all: $(TARGETS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@


clean:
	rm $(OBJECTS) $(TARGETS)