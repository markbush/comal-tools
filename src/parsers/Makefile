DEFAULT: all

CXX = c++
CXXFLAGS += -std=c++17

SOURCES = BinaryParser.cpp ParseOps.cpp ParseNames.cpp TextParser.cpp ParseCmds.cpp NameTable.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS)

%.cpp.o: $(SOURCES)
	$(CXX) -c $(CXXFLAGS) -o $^ $<

BinaryParser.o: BinaryParser.h
ParseOps.o: ParseOps.h
ParseNames.o: ParseNames.h
TextParser.o: TextParser.h
ParseCmds.o: ParseCmds.h
NameTable.o: NameTable.h
