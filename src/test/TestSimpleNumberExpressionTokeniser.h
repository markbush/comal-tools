#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"

using namespace std::string_literals;

class SimpleNumberExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testAdd(void) {
    // a + b => a b +
    std::string value1 {"123"s};
    std::string value2 {"23.45"s};
    std::string op {"+"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testNegAdd(void) {
    // - a + b => a neg b +
    std::string value1 {"12"s};
    std::string value2 {"34"s};
    std::string op {"+"s};
    std::string neg {"-"s};
    std::string expr {neg+value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 4);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "U"+neg);
    TS_ASSERT_EQUALS(tokens[2], "N"+value2);
    TS_ASSERT_EQUALS(tokens[3], "O"+op);
  }

  void testAddNeg(void) {
    // a + - b => a b neg +
    std::string value1 {"12"s};
    std::string value2 {"34"s};
    std::string op {"+"s};
    std::string neg {"-"s};
    std::string expr {value1+op+neg+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 4);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "U"+neg);
    TS_ASSERT_EQUALS(tokens[3], "O"+op);
  }

  void testMultiply(void) {
    // a * b => a b *
    std::string value1 {"123e13"s};
    std::string value2 {".45"s};
    std::string op {"*"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testSubtract(void) {
    // a - b => a b -
    std::string value1 {"0"s};
    std::string value2 {"23"s};
    std::string op {"-"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testDivide(void) {
    // a / b => a b /
    std::string value1 {"123"s};
    std::string value2 {"2"s};
    std::string op {"/"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testPower(void) {
    // a ^ b => a b ^
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"^"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testMod(void) {
    // a ^ b => a b ^
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"mod"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testDiv(void) {
    // a ^ b => a b ^
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"div"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testEq(void) {
    // a = b => a b =
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"="s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testNotEq(void) {
    // a <> b => a b <>
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"<>"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testLess(void) {
    // a < b => a b <
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"<"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testGreater(void) {
    // a > b => a b >
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {">"s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testLessEq(void) {
    // a <= b => a b <=
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {"<="s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }

  void testGreaterEq(void) {
    // a >= b => a b >=
    std::string value1 {"4"s};
    std::string value2 {"3"s};
    std::string op {">="s};
    std::string expr {value1+op+value2};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+op);
  }
};
