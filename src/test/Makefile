DEFAULT: test

CXX = c++
CXXFLAGS += -std=c++17

TARGET = runner

TESTS = *.h
HEADERS = $(TESTS)

SOURCES = runner.cpp
OBJECTS = $(SOURCES:.cpp=.o)
#LIB_OBJECTS = ../*/*.o

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $< #$(LIB_OBJECTS)

%.cpp.o: $(SOURCES)
	$(CXX) -c $(CXXFLAGS) -o $^ $<

$(SOURCES): $(HEADERS) $(LIB_OBJECTS)
	cxxtestgen --error-printer -o $@ $(TESTS)
