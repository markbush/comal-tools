#include "ExprTokeniser.h"
#include "../logger/Logger.h"

using namespace std::string_literals;

static Logger logger {"ExprTokeniser"s};

static int precedence(std::string op) {
  if (op=="Oor"s) return 2;
  if (op=="Oand"s) return 3;
  if (op=="Onot"s) return 4;
  if (op=="O="s || op=="O<>"s || op=="O<"s || op=="O>"s || op=="O<="s || op=="O>="s || op=="Oin"s) return 5;
  if (op=="O+"s || op=="O-") return 6;
  if (op=="O*"s || op=="O/"s || op=="Odiv"s || op=="Omod"s) return 7;
  if (op=="O^"s) return 8;
  return 100;
}

void ExprTokeniser::skipSpace() {
  size_t exprEnd = input_.size();
  size_t pos = 0;
  while (pos<exprEnd && input_[pos]==' ') pos++;
  input_ = input_.substr(pos, exprEnd-pos);
}

void ExprTokeniser::shuntOp() {
  std::string otherOp = operators.back();
  operators.pop_back();
  tokens.push_back(otherOp);
}

void ExprTokeniser::tokeniseOpenParen() {
  size_t inputSize = input_.size();
  operators.push_back("("s);
  input_ = input_.substr(1, inputSize-1);
}

void ExprTokeniser::tokeniseCloseParen() {
  size_t inputSize = input_.size();
  while (operators.size() > 0 && operators.back() != "(") {
    shuntOp();
  }
  if (operators.size() > 0) {
    operators.pop_back(); // must be "("
  }
  input_ = input_.substr(1, inputSize-1);
}

bool ExprTokeniser::tokeniseNumber(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string number = match[1];
  size_t matchedChars = number.size();
  if (matchedChars > 0) {
    tokens.push_back("N"+number);
    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    return true;
  }
  return false;
}

bool ExprTokeniser::tokeniseBool(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string boolValue = match[1];
  size_t matchedChars = boolValue.size();
  if (matchedChars > 0) {
    tokens.push_back("B"+boolValue);
    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    return true;
  }
  return false;
}

bool ExprTokeniser::tokeniseFunc(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string op = match[1];
  size_t matchedChars = op.size();
  if (op.size() > 0) {
    operators.push_back("F"+op);
    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    return true;
  }
  return false;
}

bool ExprTokeniser::tokeniseOp(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string op = match[1];
  size_t matchedChars = op.size();
  if (op.size() > 0) {
    int p = precedence("O"+op);

    while (operators.size() > 0 && operators.back() != "(" && precedence(operators.back()) >= p) {
      shuntOp();
    }

    operators.push_back("O"+op);
    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    return true;
  }
  return false;
}

bool ExprTokeniser::tokeniseVar(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string var = match[1];
  size_t matchedChars = var.size();
  if (matchedChars > 0) {
    tokens.push_back("V"+var);
    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    return true;
  }
  return false;
}

void ExprTokeniser::tokeniseString() {
  size_t inputSize = input_.size();
  std::string stringVal {};
  size_t pos = 1;
  while (pos < inputSize) {
    auto nextChar = input_[pos++];
    if (nextChar == '"') { // maybe end of string?
      if (pos < inputSize) { // more chars to check?
        auto nextNextChar = input_[pos];
        if (nextNextChar == '"') { // embedded quote
          stringVal += "\"\"";
          pos++;
          continue;
        }
        if (nextNextChar >='0' && nextNextChar <='9') { // embedded control char
          stringVal += '"';
          while (pos < inputSize && input_[pos] != '"') {
            stringVal += input_[pos++];
          }
          stringVal += '"';
          if (pos < inputSize) pos++;
          continue;
        }
      }
      // must be end of string
      break;
    }
    stringVal += nextChar;
  }
  tokens.push_back("S"+stringVal);
  input_ = input_.substr(pos, inputSize-pos);
}

static const std::regex regexNumber("^([0-9]*(\\.[0-9]*)?(e(\\+|-|)[0-9]+)?)", std::regex_constants::icase);
static const std::regex regexBool("^(true|false)", std::regex_constants::icase);
static const std::regex regexOperator("^(\\+|-|\\*|/|\\^|div|mod|in|=|<>|<=|>=|<|>|and|or|not)", std::regex_constants::icase);
static const std::regex regexFunc("^(abs|ord|atn|chr\\$|cos|exp|int|len|log|rnd|sgn|sin|spc\\$|sqr|tan)", std::regex_constants::icase);
static const std::regex regexVar("^([a-z][a-z0-9'\\\\\\[\\]_]*(#|\\$|))");

// tokenise using shunting yard algorithm
void ExprTokeniser::tokenise() {
  std::smatch match;
  skipSpace();
  size_t inputSize = input_.size();
  if (inputSize == 0) return;

  if (input_[0] == '(') { tokeniseOpenParen(); return; }
  if (input_[0] == ')') { tokeniseCloseParen(); return; }
  if (std::regex_search(input_, match, regexNumber)) { if (tokeniseNumber(match)) return; }
  if (std::regex_search(input_, match, regexBool)) { if (tokeniseBool(match)) return; }
  if (std::regex_search(input_, match, regexFunc)) { if (tokeniseFunc(match)) return; }
  if (std::regex_search(input_, match, regexOperator)) { if (tokeniseOp(match)) return; }
  if (std::regex_search(input_, match, regexVar)) { if (tokeniseVar(match)) return; }
  if (input_[0] == '"') { tokeniseString(); return; }

  logger.warn("FAILED: <{}>", input_);
  tokens.push_back("null"s);
  input_ = "";
}

std::vector<std::string> ExprTokeniser::getTokens() {
  if (tokens.size() == 0) {
    while (input_.size() > 0) {
      tokenise();
    }
    while (operators.size() > 0) {
      shuntOp();
    }
  }

  return tokens;
}
