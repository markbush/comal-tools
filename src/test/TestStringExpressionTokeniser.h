#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"

using namespace std::string_literals;

class StringExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testString(void) {
    // "Hello, world" => Hello, world
    std::string expr {"\"Hello, world\""s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "SHello, world");
  }

  void testStringConcat(void) {
    // "Hello" + ", " + "world" => Hello , + world +
    std::string expr {"\"Hello\"+\", \"+\"world\""s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "SHello");
    TS_ASSERT_EQUALS(tokens[1], "S, ");
    TS_ASSERT_EQUALS(tokens[2], "O+");
    TS_ASSERT_EQUALS(tokens[3], "Sworld");
    TS_ASSERT_EQUALS(tokens[4], "O+");
  }

  void testNullString(void) {
    // ""0"" => "0"
    std::string expr {"\"\"0\"\""s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "S\"0\"");
  }

  void testStringWithQuote(void) {
    // "ab""cd" => ab""cd
    std::string expr {"\"ab\"\"cd\""s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "Sab\"\"cd");
  }
};
