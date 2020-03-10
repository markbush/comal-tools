#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"
#include "TestConstants.hxx"

class BooleanExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testAnd(void) {
    // a and b => a b and
    std::string expr {trueVal+" "+andOp+" "+falseVal};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[1], "B"+falseVal);
    TS_ASSERT_EQUALS(tokens[2], "O"+andOp);
  }

  void testOr(void) {
    // a or b => a b or
    std::string expr {trueVal+" "+orOp+" "+falseVal};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 3);
    TS_ASSERT_EQUALS(tokens[0], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[1], "B"+falseVal);
    TS_ASSERT_EQUALS(tokens[2], "O"+orOp);
  }

  void testAndOr(void) {
    // a and b or c => a b and c or
    std::string expr {trueVal+" "+andOp+" "+falseVal+orOp+" "+trueVal};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[1], "B"+falseVal);
    TS_ASSERT_EQUALS(tokens[2], "O"+andOp);
    TS_ASSERT_EQUALS(tokens[3], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[4], "O"+orOp);
  }

  void testOrAnd(void) {
    // a or b and c => a b c and or
    std::string expr {trueVal+" "+orOp+" "+falseVal+andOp+" "+trueVal};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 5);
    TS_ASSERT_EQUALS(tokens[0], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[1], "B"+falseVal);
    TS_ASSERT_EQUALS(tokens[2], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[3], "O"+andOp);
    TS_ASSERT_EQUALS(tokens[4], "O"+orOp);
  }

  void testNot(void) {
    // not a => a not
    std::string expr {notOp+" "+falseVal};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 2);
    TS_ASSERT_EQUALS(tokens[0], "B"+falseVal);
    TS_ASSERT_EQUALS(tokens[1], "O"+notOp);
  }

  void testNotAnd(void) {
    // not a and b => a not b and
    std::string expr {notOp+" "+falseVal+" "+andOp+" "+trueVal};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 4);
    TS_ASSERT_EQUALS(tokens[0], "B"+falseVal);
    TS_ASSERT_EQUALS(tokens[1], "O"+notOp);
    TS_ASSERT_EQUALS(tokens[2], "B"+trueVal);
    TS_ASSERT_EQUALS(tokens[3], "O"+andOp);
  }

  void testLessAndGreaterEq(void) {
    // a < b and c >= d => a b < c d >= and
    std::string expr {value1+" "+lessOp+" "+value2+" "+andOp+" "+value3+" "+greaterEqOp+" "+value4};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 7);
    TS_ASSERT_EQUALS(tokens[0], "N"+value1);
    TS_ASSERT_EQUALS(tokens[1], "N"+value2);
    TS_ASSERT_EQUALS(tokens[2], "O"+lessOp);
    TS_ASSERT_EQUALS(tokens[3], "N"+value3);
    TS_ASSERT_EQUALS(tokens[4], "N"+value4);
    TS_ASSERT_EQUALS(tokens[5], "O"+greaterEqOp);
    TS_ASSERT_EQUALS(tokens[6], "O"+andOp);
  }
};
