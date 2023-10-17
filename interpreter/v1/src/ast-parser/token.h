#pragma once
#include <string>

using namespace std;

struct Span {
  int line;
  int column;
};

enum TokenType {
  DELIMETER,
  SPECIAL_FORM,
  IDENTIFIER,
  LITERAL,
};

struct Token {
  TokenType type;
  string value;
  Span span;
};
