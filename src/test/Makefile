DEFAULT: test

CXX = c++
CXXFLAGS += -std=c++17

TARGET = runner

TESTS = *.h
HEADERS = $(TESTS)

SOURCES = runner.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LIB_OBJECTS = ../parsers/*.o

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB_OBJECTS)

%.cpp.o: $(SOURCES)
	$(CXX) -c $(CXXFLAGS) -o $^ $<

$(SOURCES): $(HEADERS)
	cxxtestgen --error-printer -o $@ $(TESTS)
