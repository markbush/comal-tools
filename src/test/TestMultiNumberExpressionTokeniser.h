#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"
#include "TestConstants.hxx"

class MultiNumberExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testAddSubtract(void) {
    // a + b - c => a b + c -
    std::string expr {value1+addOp+value2+subOp+value3};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+addOp);
    TS_ASSERT_EQUALS(tokens[3], "N"+value3);
    TS_ASSERT_EQUALS(tokens[4], "O"+subOp);
  }

  void testAddPower(void) {
    // a + b ^ c => a b c ^ +
    std::string expr {value1+addOp+value2+powOp+value3};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "N"+value3);
    TS_ASSERT_EQUALS(tokens[3], "O"+powOp);
    TS_ASSERT_EQUALS(tokens[4], "O"+addOp);
  }

  void testPowerAdd(void) {
    // a ^ b + c => a b ^ c +
    std::string expr {value1+powOp+value2+addOp+value3};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+powOp);
    TS_ASSERT_EQUALS(tokens[3], "N"+value3);
    TS_ASSERT_EQUALS(tokens[4], "O"+addOp);
  }

  void testAddMul(void) {
    // a + b * c => a b c * +
    std::string expr {value1+addOp+value2+mulOp+value3};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "N"+value3);
    TS_ASSERT_EQUALS(tokens[3], "O"+mulOp);
    TS_ASSERT_EQUALS(tokens[4], "O"+addOp);
  }

  void testDivAdd(void) {
    // a / b + c => a b / c +
    std::string expr {value1+divOp+value2+addOp+value3};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+divOp);
    TS_ASSERT_EQUALS(tokens[3], "N"+value3);
    TS_ASSERT_EQUALS(tokens[4], "O"+addOp);
  }

  void testDivAddEqualPower(void) {
    // a / b + c = d ^ e => a b / c + d e ^ =
    std::string expr {value1+divOp+value2+addOp+value3+eqOp+value4+powOp+value5};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 9);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+divOp);
    TS_ASSERT_EQUALS(tokens[3], "N"+value3);
    TS_ASSERT_EQUALS(tokens[4], "O"+addOp);
    TS_ASSERT_EQUALS(tokens[5], "N"+value4);
    TS_ASSERT_EQUALS(tokens[6], "N"+value5);
    TS_ASSERT_EQUALS(tokens[7], "O"+powOp);
    TS_ASSERT_EQUALS(tokens[8], "O"+eqOp);
  }
};
