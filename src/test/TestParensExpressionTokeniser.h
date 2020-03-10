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
    // (a + b) * c => a b + c *
    std::string expr {"(12 + 34) * 56"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N12");
    TS_ASSERT_EQUALS(tokens[1], "N34");
    TS_ASSERT_EQUALS(tokens[2], "O"+addOp);
    TS_ASSERT_EQUALS(tokens[3], "N56");
    TS_ASSERT_EQUALS(tokens[4], "O"+mulOp);
  }

  void testNegAdd(void) {
    // -(a + b) => a b + neg
    std::string expr {"-(12 + 34)"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 4);
    TS_ASSERT_EQUALS(tokens[0], "N12");
    TS_ASSERT_EQUALS(tokens[1], "N34");
    TS_ASSERT_EQUALS(tokens[2], "O"+addOp);
    TS_ASSERT_EQUALS(tokens[3], "U-");
  }
};
