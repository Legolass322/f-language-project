#include "parse-node-funcs.h"
#include "astnode.h"
#include "token.h"
#include <any>
#include <iostream>
#include <map>

ListASTNode *parseListASTNode(vector<any> ast) {
  vector<ASTNode *> children;

  for (auto i : ast) {
    if (i.type() == typeid(vector<any>)) {
      children.push_back(parseASTNode(any_cast<vector<any>>(i)));
    } else {
      children.push_back(new ASTNode(any_cast<Token>(i)));
    }
  }

  return new ListASTNode(children);
}

FuncDeclASTNode *parseFuncDeclASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  ASTNode *func_name = new ASTNode(any_cast<Token>(ast[1]));
  ListASTNode *params = parseListASTNode(any_cast<vector<any>>(ast[2]));
  ListASTNode *body = parseListASTNode(any_cast<vector<any>>(ast[3]));

  vector<ASTNode *> children = {func_name, params, body};

  return new FuncDeclASTNode(token, children);
}

SetqASTNode *parseSetqASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  ASTNode *var_name = new ASTNode(any_cast<Token>(ast[1]));
  ASTNode *init;

  if (ast[2].type() == typeid(vector<any>)) {
    init = parseASTNode(any_cast<vector<any>>(ast[2]));
  } else {
    init = new ASTNode(any_cast<Token>(ast[2]));
  }

  vector<ASTNode *> children = {var_name, init};

  return new SetqASTNode(token, children);
}

WhileASTNode *parseWhileASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  ASTNode *condition = parseASTNode(any_cast<vector<any>>(ast[1]));
  ListASTNode *body = parseListASTNode(any_cast<vector<any>>(ast[2]));

  vector<ASTNode *> children = {condition, body};

  return new WhileASTNode(token, children);
}

LambdaASTNode *parseLambdaASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  ListASTNode *params = parseListASTNode(any_cast<vector<any>>(ast[1]));
  ListASTNode *body = parseListASTNode(any_cast<vector<any>>(ast[2]));

  vector<ASTNode *> children = {params, body};

  return new LambdaASTNode(token, children);
}

ASTNode *parseASTNode(vector<any> ast) {

  if (ast.size() == 0)
    return new ListASTNode();

  if (ast[0].type() == typeid(vector<any>))
    return parseListASTNode(any_cast<vector<any>>(ast));

  Token token = any_cast<Token>(ast[0]);
  vector<ASTNode *> children;

  switch (token.type) {
  case SPECIAL_FORM:
    if (token.value == "func") {
      return parseFuncDeclASTNode(ast);
    } else if (token.value == "setq") {
      return parseSetqASTNode(ast);
    } else if (token.value == "while") {
      return parseWhileASTNode(ast);
    } else if (token.value == "lambda") {
      return parseLambdaASTNode(ast);
    } else {
      for (int i = 1; i < ast.size(); i++) {
        if (ast[i].type() == typeid(vector<any>)) {
          children.push_back(parseASTNode(any_cast<vector<any>>(ast[i])));
        } else {
          children.push_back(new ASTNode(any_cast<Token>(ast[i])));
        }
      }

      return new ASTNode(token, children);
    }
  case IDENTIFIER:
    for (int i = 1; i < ast.size(); i++) {
      if (ast[i].type() == typeid(vector<any>)) {
        children.push_back(parseASTNode(any_cast<vector<any>>(ast[i])));
      } else {
        children.push_back(new ASTNode(any_cast<Token>(ast[i])));
      }
    }

    return new FuncCallASTNode(token, children);
  case LITERAL:
    return parseListASTNode(ast);
  default:
    throw "Invalid token type";
  }
}

tuple<vector<any>, int> parseVector(vector<Token> tokens, int start) {
  vector<any> ast;

  int depth = 1;

  for (int i = start; i < tokens.size(); i++) {
    Token token = tokens[i];

    start = i;

    if (depth == 0)
      break;

    if (token.type == DELIMETER) {
      if (token.value == "(") {
        auto [child_ast, new_start] = parseVector(tokens, i + 1);
        ast.push_back(child_ast);
        i = new_start - 1;
      } else
        depth--;
    } else
      ast.push_back(token);
  }

  return {ast, start};
}

vector<any> parseTokensToVector(vector<Token> tokens) {
  vector<any> ast;
  int start = 1;

  while (start <= tokens.size()) {

    auto [child_ast, new_start] = parseVector(tokens, start);

    ast.push_back(child_ast);

    start = new_start + 1;
  }

  return ast;
}

ASTNode *parseVectorToASTNode(vector<any> ast) {
  vector<ASTNode *> children;

  for (auto i : ast) {
    children.push_back(parseASTNode(any_cast<vector<any>>(i)));
  }

  return new ASTNode({SPECIAL_FORM, "PROGRAM", {0, 0}}, children);
}

void parseTokens(vector<Token> &tokens) {
  string s;
  map<string, TokenType> token_types = {{"del", DELIMETER},
                                        {"sf", SPECIAL_FORM},
                                        {"identifier", IDENTIFIER},
                                        {"literal", LITERAL}};
  vector<string> token_types_string = {"DELIMETER", "SPECIAL_FORM",
                                       "IDENTIFIER", "LITERAL"};

  while (getline(cin, s)) {
    string token_value = s.substr(0, s.find(" "));
    string token_span_string =
        s.substr(s.find(" ") + 1, s.find(" ", s.find(" ") + 1));
    string token_type_string = s.substr(s.find(" ", s.find(" ") + 1) + 1);

    // error handling
    if (token_span_string == "ERROR") {
      Token last = tokens[tokens.size() - 1];
      cout << "ERROR during lexer parsing at " << last.span.line << ":"
           << last.span.column << endl;
      cout << last.value << " " << token_types_string[last.type] << endl;
      exit(1);
    }

    // skip log messages
    if (token_value == "LOG_PREFIX")
      continue;

    TokenType token_type = token_types[token_type_string];

    Span token_span = {
        stoi(token_span_string.substr(0, token_span_string.find(":"))),
        stoi(token_span_string.substr(token_span_string.find(":") + 1))};

    Token token = {token_type, token_value, token_span};

    tokens.push_back(token);
  }
}
