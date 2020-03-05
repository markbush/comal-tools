DEFAULT: all

CXX = c++
CXXFLAGS += -std=c++17

SOURCES = BinaryParser.cpp ParseOps.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS)

%.cpp.o: $(SOURCES)
	$(CXX) -c $(CXXFLAGS) -o $^ $<

BinaryParser.o: BinaryParser.h
ParseOps.o: ParseOps.h
