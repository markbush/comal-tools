#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"
#include "TestConstants.hxx"

class ParensExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testAddMul(void) {
    // a + b * c => a b c * +
    std::string expr {"12 + 34 * 56"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N12");
    TS_ASSERT_EQUALS(tokens[1], "N34");
    TS_ASSERT_EQUALS(tokens[2], "N56");
    TS_ASSERT_EQUALS(tokens[3], "O"+mulOp);
    TS_ASSERT_EQUALS(tokens[4], "O"+addOp);
  }

  void testAddMulParens(void) {
    // (a + b) * c => a b + G c *
    std::string expr {"(12 + 34) * 56"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 6);
    TS_ASSERT_EQUALS(tokens[0], "N12");
    TS_ASSERT_EQUALS(tokens[1], "N34");
    TS_ASSERT_EQUALS(tokens[2], "O"+addOp);
    TS_ASSERT_EQUALS(tokens[3], "G");
    TS_ASSERT_EQUALS(tokens[4], "N56");
    TS_ASSERT_EQUALS(tokens[5], "O"+mulOp);
  }

  void testNegAdd(void) {
    // -(a + b) => a b + G neg
    std::string expr {"-(12 + 34)"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N12");
    TS_ASSERT_EQUALS(tokens[1], "N34");
    TS_ASSERT_EQUALS(tokens[2], "O"+addOp);
    TS_ASSERT_EQUALS(tokens[3], "G");
    TS_ASSERT_EQUALS(tokens[4], "U-");
  }
};
