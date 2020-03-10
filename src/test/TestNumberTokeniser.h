#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"

using namespace std::string_literals;

class NumberTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testnumber(void) {
    std::string number {"123"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testReal(void) {
    std::string number {"123.45"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealOnlyFraction(void) {
    std::string number {".45"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testnumberPositiveExponent(void) {
    std::string number {"123e+4"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealPositiveExponent(void) {
    std::string number {"123.45e+12"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealOnlyFractionPositiveExponent(void) {
    std::string number {".45e+76"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testnumberNegativeExponent(void) {
    std::string number {"123e-4"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealNegativeExponent(void) {
    std::string number {"123.45e-12"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealOnlyFractionNegativeExponent(void) {
    std::string number {".45e-76"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testnumberUnsignedExponent(void) {
    std::string number {"123e4"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealUnsignedExponent(void) {
    std::string number {"123.45e12"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }

  void testRealOnlyFractionUnsignedExponent(void) {
    std::string number {".45e76"s};
    ExprTokeniser tokeniser {number};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "N"+number);
  }
};
