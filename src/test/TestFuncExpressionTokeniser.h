#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include "../parsers/ExprTokeniser.h"
#include "TestConstants.hxx"

class FuncExprTokeniserTestSuite : public CxxTest::TestSuite {
public:
  void testAbs(void) {
    // abs(a) => a abs
    std::string expr {"abs(3)"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 2);
    TS_ASSERT_EQUALS(tokens[0], "N3");
    TS_ASSERT_EQUALS(tokens[1], "Fabs");
  }

  void testChr(void) {
    // chr$(a) => a chr$
    std::string expr {"chr$(3)"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 2);
    TS_ASSERT_EQUALS(tokens[0], "N3");
    TS_ASSERT_EQUALS(tokens[1], "Fchr$");
  }

  void testMultiFuncs(void) {
    // abs(3) * (4 + int(3.14)) * sgn(3.14) => 3 abs 4 3.14 int + * 3.14 sgn *
    std::string expr {"abs(3) * (4 + int(3.14)) * sgn(3.14)"s};
    ExprTokeniser tokeniser {expr};

    std::vector<std::string> tokens = tokeniser.getTokens();

    TS_ASSERT_EQUALS(tokens.size(), 10);
    TS_ASSERT_EQUALS(tokens[0], "N3");
    TS_ASSERT_EQUALS(tokens[1], "Fabs");
    TS_ASSERT_EQUALS(tokens[2], "N4");
    TS_ASSERT_EQUALS(tokens[3], "N3.14");
    TS_ASSERT_EQUALS(tokens[4], "Fint");
    TS_ASSERT_EQUALS(tokens[5], "O+");
    TS_ASSERT_EQUALS(tokens[6], "O*");
    TS_ASSERT_EQUALS(tokens[7], "N3.14");
    TS_ASSERT_EQUALS(tokens[8], "Fsgn");
    TS_ASSERT_EQUALS(tokens[9], "O*");
  }
};
