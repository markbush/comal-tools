DEFAULT: all

CXX = c++
CXXFLAGS += -std=c++17

TARGET_D = comald
TARGET_C = comalc
SUBDIRS = parsers

TARGETS = $(TARGET_D) $(TARGET_C)
HEADERS = */*.h

SOURCES_D = comald.cpp
OBJECTS_D = $(SOURCES_D:.cpp=.o)
LIB_OBJECTS_D = parsers/BinaryParser.o parsers/ParseOps.o parsers/ParseNames.o

SOURCES_C = comalc.cpp
OBJECTS_C = $(SOURCES_C:.cpp=.o)
LIB_OBJECTS_C = parsers/TextParser.o parsers/ParseCmds.o parsers/NameTable.o parsers/ExprTokeniser.o

SOURCES = $(SOURCES_D) $(SOURCES_C)
OBJECTS = $(OBJECTS_D) $(OBJECTS_C)
LIB_OBJECTS = $(LIB_OBJECTS_D) $(LIB_OBJECTS_C)
LIB_SOURCES = $(LIB_OBJECTS:.o=.cpp) */*.h

TEST_TARGET = test/runner

all:
	make subdirs
	make $(TARGETS)

$(TARGET_D): $(OBJECTS_D) $(LIB_OBJECTS_D)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB_OBJECTS_D)

$(TARGET_C): $(OBJECTS_C) $(LIB_OBJECTS_C)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB_OBJECTS_C)

subdirs:
	for d in $(SUBDIRS); do (cd $$d ; make); done

%.cpp.o: $(SOURCES)
	$(CXX) -c $(CXXFLAGS) -o $^ $<

$(OBJECTS): $(HEADERS)

test: $(LIB_SOURCES)
	cd test ; make

clean:
	rm -f $(TARGETS) $(OBJECTS) $(LIB_OBJECTS)
