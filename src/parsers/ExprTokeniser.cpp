#include "ExprTokeniser.h"
#include "../logger/Logger.h"

using namespace std::string_literals;

static Logger logger {"ExprTokeniser"s};

static uint8_t toLower(uint8_t c) {
  if (c>=65 && c<=90) return c+32;
  return c;
}

static std::string toLower(std::string value) {
  std::string result {};
  for (auto& c : value) {
    result += toLower(c);
  }
  return result;
}

static int precedence(std::string op) {
  if (op == "Ccomma"s) return 1;
  if (op=="Oor"s) return 2;
  if (op=="Oand"s) return 3;
  if (op=="O="s || op=="O<>"s || op=="O<"s || op=="O>"s || op=="O<="s || op=="O>="s || op=="Oin"s) return 4;
  if (op=="O+"s || op=="O-") return 5;
  if (op=="O*"s || op=="O/"s || op=="Odiv"s || op=="Omod"s) return 6;
  if (op=="O^"s) return 7;
  if (op=="U-"s || op=="U+"s || op=="Unot"s) return 8;
  return 100;
}

void ExprTokeniser::skipSpace() {
  size_t exprEnd = input_.size();
  size_t pos = 0;
  while (pos<exprEnd && input_[pos]==' ') pos++;
  input_ = input_.substr(pos, exprEnd-pos);
}

void ExprTokeniser::shuntOp() {
  std::string op = operators.back();
  operators.pop_back();
  if (op == "Frnd(") {
    tokens.push_back("Crnd"s);
  }
  tokens.push_back(op);
}

void ExprTokeniser::shuntComma() {
  if (operators.size() > 0 && operators.back() == "Ccomma") {
    shuntOp();
  }
}

void ExprTokeniser::tokeniseOpenParen() {
  size_t inputSize = input_.size();
  operators.push_back("("s);
  input_ = input_.substr(1, inputSize-1);
  unaryAllowed = true;
}

void ExprTokeniser::tokeniseCloseParen() {
  size_t inputSize = input_.size();
  while (operators.size() > 0 && operators.back() != "(") {
    shuntOp();
  }
  if (operators.size() > 0) {
    operators.pop_back(); // must be "("
  }
  if (operators.size() == 0 || (operators.size() > 0 && operators.back()[0] != 'F')) {
    tokens.push_back("G"s); // preserve grouping
  }
  input_ = input_.substr(1, inputSize-1);
}

bool ExprTokeniser::tokeniseNumber(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string number = match[1];
  size_t matchedChars = number.size();
  if (matchedChars > 0) {
    shuntComma();
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
    shuntComma();
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
  if (toLower(op) == "rnd(") {
    matchedChars--;
  }
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
    if (op == ",") {
      op = "Ccomma"s;
    } else {
      op = "O"+op;
    }
    int p = precedence(op);

    while (operators.size() > 0 && operators.back() != "(" && precedence(operators.back()) >= p) {
      shuntOp();
    }

    operators.push_back(op);

    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    unaryAllowed = true;
    return true;
  }
  return false;
}

void ExprTokeniser::tokeniseUnary() {
  size_t inputSize = input_.size();
  auto first = input_[0];
  if (first == '-' || first == '+' || (inputSize >= 3 && toLower(input_.substr(0, 3)) == "not")) {
    std::string op {"Unot"s};
    if (first == '-' || first == '+') op = "U"s + first;
    size_t matchedChars = op.size()-1;
    int p = precedence(op);

    while (operators.size() > 0 && operators.back() != "(" && precedence(operators.back()) >= p) {
      shuntOp();
    }

    operators.push_back(op);
    input_ = input_.substr(matchedChars, inputSize-matchedChars);
    unaryAllowed = true;
  }
}

bool ExprTokeniser::tokeniseVar(std::smatch& match) {
  size_t inputSize = input_.size();
  std::string var = match[1];
  size_t matchedChars = var.size();
  if (matchedChars > 0) {
    shuntComma();
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
  shuntComma();
  tokens.push_back("S"+stringVal);
  input_ = input_.substr(pos, inputSize-pos);
}

static const std::regex regexNumber("^([0-9]*(\\.[0-9]*)?(e(\\+|-|)[0-9]+)?)", std::regex_constants::icase);
static const std::regex regexBool("^(true|false)", std::regex_constants::icase);
static const std::regex regexOperator("^(,|\\+|-|\\*|/|\\^|div|mod|in|=|<>|<=|>=|<|>|and|or|not)", std::regex_constants::icase);
static const std::regex regexFunc("^(abs|ord|atn|chr\\$|cos|eod|eof|esc|exp|int|key\\$|len|log|rnd\\(|rnd|sgn|sin|spc\\$|sqr|str\\$|tan|zone|status\\$|time)", std::regex_constants::icase);
static const std::regex regexVar("^([a-z][a-z0-9'\\\\\\[\\]_]*(#|\\$|))");

// tokenise using shunting yard algorithm
void ExprTokeniser::tokenise() {
  skipSpace();
  std::smatch match;
  size_t inputSize = input_.size();
  if (inputSize == 0) return;
  unaryAllowed = false;

  if (input_[0] == '(') { tokeniseOpenParen(); return; }
  if (input_[0] == ')') { tokeniseCloseParen(); return; }
  if (std::regex_search(input_, match, regexNumber)) { if (tokeniseNumber(match)) return; }
  if (std::regex_search(input_, match, regexBool)) { if (tokeniseBool(match)) return; }
  if (std::regex_search(input_, match, regexFunc)) { if (tokeniseFunc(match)) return; }
  if (std::regex_search(input_, match, regexOperator)) { if (tokeniseOp(match)) return; }
  if (std::regex_search(input_, match, regexVar)) { if (tokeniseVar(match)) return; }
  if (input_[0] == '"') { tokeniseString(); return; }

  logger.warn("FAILED: <{}>", input_);
  //tokens.push_back("null"s);
  input_ = "";
}

std::vector<std::string> ExprTokeniser::getTokens() {
  // TODO stop parsing at end of valid expression
  if (tokens.size() == 0) {
    while (input_.size() > 0) {
      skipSpace();
      if (input_[0] == ';') break; // end of expression
      if (input_[0] == '/' && input_.size() > 1 && input_[1] == '/') break; // comment
      if (unaryAllowed) {
        tokeniseUnary();
      }
      tokenise();
    }
    while (operators.size() > 0) {
      shuntOp();
    }
  }

  return tokens;
}
