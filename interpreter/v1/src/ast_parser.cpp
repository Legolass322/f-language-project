#include <any>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

// class, struct, enum declarations
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

class ASTNode {
public:
  Token token;
  vector<ASTNode *> children;

  ASTNode() {}

  ASTNode(vector<ASTNode *> children) { this->children = children; }

  ASTNode(Token token) { this->token = token; }

  ASTNode(Token token, vector<ASTNode *> children) {
    this->token = token;
    this->children = children;
  }

  virtual ~ASTNode() {
    for (auto i : this->children) {
      delete i;
    }
  }

  virtual void print(int depth = 0) {
    for (int i = 0; i < depth; i++)
      cout << "-";

    if (this->token.value != "")
      cout << this->token.value << endl;
    else
      cout << "List" << endl;

    if (children.size() > 0) {
      for (int i = 0; i < depth; i++)
        cout << "-";
      cout << " Children: " << endl;

      for (auto i : this->children) {
        i->print(depth + 1);
      }
    }
  }
};

class ListASTNode : public ASTNode {
public:
  ListASTNode() {}
  ListASTNode(vector<ASTNode *> children) : ASTNode(children) {}
};

class FuncDeclASTNode : public ASTNode {
public:
  ListASTNode *params;
  ListASTNode *body;
  Token func_name;
  FuncDeclASTNode(Token token, vector<ASTNode *> children)
      : ASTNode(token, children) {
    this->func_name = children[0]->token;
    this->params = static_cast<ListASTNode *>(children[1]);
    this->body = static_cast<ListASTNode *>(children[2]);
  }
};

class FuncCallASTNode : public ASTNode {
public:
  vector<ASTNode *> args;
  Token func_name;
  FuncCallASTNode(Token token, vector<ASTNode *> children)
      : ASTNode(token, children) {
    this->args = children;
  }
};

class SetqASTNode : public ASTNode {
public:
  Token var_name;
  ASTNode *value;
  SetqASTNode(Token token, vector<ASTNode *> children)
      : ASTNode(token, children) {
    this->var_name = children[0]->token;
    this->value = children[1];
  }
};

class WhileASTNode : public ASTNode {
public:
  ASTNode *condition;
  ListASTNode *body;
  WhileASTNode(Token token, vector<ASTNode *> children)
      : ASTNode(token, children) {
    this->condition = children[0];
    this->body = static_cast<ListASTNode *>(children[1]);
  }
};

class LambdaASTNode : public ASTNode {
public:
  ListASTNode *params;
  ListASTNode *body;
  LambdaASTNode(Token token, vector<ASTNode *> children)
      : ASTNode(token, children) {
    this->params = static_cast<ListASTNode *>(children[0]);
    this->body = static_cast<ListASTNode *>(children[1]);
  }
};
// function declarations
ASTNode *parseASTNode(vector<any> ast);
ListASTNode *parseListASTNode(vector<any> ast);
FuncDeclASTNode *parseFuncDeclASTNode(vector<any> ast);
SetqASTNode *parseSetqASTNode(vector<any> ast);
WhileASTNode *parseWhileASTNode(vector<any> ast);
LambdaASTNode *parseLambdaASTNode(vector<any> ast);
tuple<vector<any>, int> parseTokensToVector(vector<Token> tokens, int start);

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
  vector<ASTNode *> children;

  children.push_back(new ASTNode(any_cast<Token>(ast[1])));
  children.push_back(parseListASTNode(any_cast<vector<any>>(ast[2])));
  children.push_back(parseListASTNode(any_cast<vector<any>>(ast[3])));

  return new FuncDeclASTNode(token, children);
}

SetqASTNode *parseSetqASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  vector<ASTNode *> children;

  children.push_back(new ASTNode(any_cast<Token>(ast[1])));

  if (ast[2].type() == typeid(vector<any>)) {
    children.push_back(parseASTNode(any_cast<vector<any>>(ast[2])));
  } else {
    children.push_back(new ASTNode(any_cast<Token>(ast[2])));
  }

  return new SetqASTNode(token, children);
}

WhileASTNode *parseWhileASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  vector<ASTNode *> children;

  children.push_back(parseASTNode(any_cast<vector<any>>(ast[1])));
  children.push_back(parseListASTNode(any_cast<vector<any>>(ast[2])));

  return new WhileASTNode(token, children);
}

LambdaASTNode *parseLambdaASTNode(vector<any> ast) {
  Token token = any_cast<Token>(ast[0]);
  vector<ASTNode *> children;

  children.push_back(parseListASTNode(any_cast<vector<any>>(ast[1])));
  children.push_back(parseListASTNode(any_cast<vector<any>>(ast[2])));

  return new LambdaASTNode(token, children);
}

ASTNode *parseASTNode(vector<any> ast) {

  if (ast.size() == 0)
    return new ListASTNode();

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

    return new ASTNode(token, children);
  case LITERAL:
    return parseListASTNode(ast);
  default:
    throw "Invalid token type";
  }
}

tuple<vector<any>, int> parseTokensToVector(vector<Token> tokens,
                                            int start = 1) {
  vector<any> ast;

  int depth = 1;

  for (int i = start; i < tokens.size(); i++) {
    Token token = tokens[i];

    start = i;

    if (depth == 0)
      break;

    if (token.type == DELIMETER) {
      if (token.value == "(") {
        auto [child_ast, new_start] = parseTokensToVector(tokens, i + 1);
        ast.push_back(child_ast);
        i = new_start - 1;
      } else
        depth--;
    } else
      ast.push_back(token);
  }

  return {ast, start};
}

void parseTokens(vector<Token> &tokens) {
  string s;
  map<string, TokenType> token_types = {{"got", DELIMETER},
                                        {"sf", SPECIAL_FORM},
                                        {"identifier", IDENTIFIER},
                                        {"literal", LITERAL}};

  while (getline(cin, s)) {
    string token_value = s.substr(0, s.find(" "));
    string token_span_string =
        s.substr(s.find(" ") + 1, s.find(" ", s.find(" ") + 1));
    string token_type_string = s.substr(s.find(" ", s.find(" ") + 1) + 1);

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

int main() {
  vector<Token> tokens;

  parseTokens(tokens);

  auto [ast, st] = parseTokensToVector(tokens);

  ASTNode *root = parseASTNode(ast);

  cout << "AST:" << endl;
  root->print();

  return 0;
}
