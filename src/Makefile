DEFAULT: all

CXX = c++
CXXFLAGS += -std=c++17

TARGET = comald
SUBDIRS = parsers

SOURCES = comald.cpp
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = */*.h
LIB_OBJECTS = parsers/BinaryParser.o parsers/ParseOps.o parsers/ParseNames.o

all:
	make subdirs
	make $(TARGET)

$(TARGET): $(OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB_OBJECTS)

subdirs:
	for d in $(SUBDIRS); do (cd $$d ; make); done

%.cpp.o: $(SOURCES)
	$(CXX) -c $(CXXFLAGS) -o $^ $<

$(OBJECTS): $(HEADERS)

test: $(TARGET)
	cd test ; make

clean:
	rm -f $(TARGET) $(OBJECTS) $(LIB_OBJECTS)
