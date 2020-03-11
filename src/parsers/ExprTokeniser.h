#ifndef EXPR_TOKENISER_H
#define EXPR_TOKENISER_H

#include <string>
#include <vector>
#include <regex>

class ExprTokeniser {
protected:
  std::string input_;
  std::vector<std::string> tokens {};
  std::vector<std::string> operators {};
  bool unaryAllowed {true};
  void skipSpace();
  void tokeniseOpenParen();
  void tokeniseCloseParen();
  bool tokeniseNumber(std::smatch& match);
  bool tokeniseBool(std::smatch& match);
  bool tokeniseFunc(std::smatch& match);
  bool tokeniseOp(std::smatch& match);
  void tokeniseUnary();
  bool tokeniseVar(std::smatch& match);
  void tokeniseString();
  void shuntOp();
  void shuntComma();
  void tokenise();
public:
  ExprTokeniser(std::string input) : input_ {input} {}
  std::vector<std::string> getTokens();
};

#endif
