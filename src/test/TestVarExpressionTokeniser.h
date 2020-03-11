#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"

using namespace std::string_literals;

class VarExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testVarReal(void) {
    // abc'd2 => abc'd2
    std::string expr {"abc'd2"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "V"+expr);
  }

  void testVarInt(void) {
    // abc1# => abc1#
    std::string expr {"abc1#"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "V"+expr);
  }

  void testVarString(void) {
    // abc1$ => abc1$
    std::string expr {"abc1$"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 1);
    TS_ASSERT_EQUALS(tokens[0], "V"+expr);
  }

  void testVarAdd(void) {
    // a + b => a b +
    std::string expr {"a+b"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "Va");
    TS_ASSERT_EQUALS(tokens[1], "Vb");
    TS_ASSERT_EQUALS(tokens[2], "O+");
  }

  void testIntVarAdd(void) {
    // a# + b# => a# b# +
    std::string expr {"a#+b#"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "Va#");
    TS_ASSERT_EQUALS(tokens[1], "Vb#");
    TS_ASSERT_EQUALS(tokens[2], "O+");
  }

  void testStringVarAdd(void) {
    // a$ + b$ => a$ b$ +
    std::string expr {"a$+b$"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "Va$");
    TS_ASSERT_EQUALS(tokens[1], "Vb$");
    TS_ASSERT_EQUALS(tokens[2], "O+");
  }

  void testVarsAndNums(void) {
    // (a + 3 * b) - c => a 3 b * + G c -
    std::string expr {"(a + 3 * b) - c"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 8);
    TS_ASSERT_EQUALS(tokens[0], "Va");
    TS_ASSERT_EQUALS(tokens[1], "N3");
    TS_ASSERT_EQUALS(tokens[2], "Vb");
    TS_ASSERT_EQUALS(tokens[3], "O*");
    TS_ASSERT_EQUALS(tokens[4], "O+");
    TS_ASSERT_EQUALS(tokens[5], "G");
    TS_ASSERT_EQUALS(tokens[6], "Vc");
    TS_ASSERT_EQUALS(tokens[7], "O-");
  }
};
