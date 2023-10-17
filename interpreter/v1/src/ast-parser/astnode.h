#pragma once
#include "token.h"
#include <vector>

using namespace std;

// ASTNode class declarations
class ASTNode {
public:
  Token token;
  vector<ASTNode *> children;

  ASTNode();
  ASTNode(vector<ASTNode *> children);
  ASTNode(Token token);
  ASTNode(Token token, vector<ASTNode *> children);

  virtual ~ASTNode();

  virtual void print(int depth = 0);
};

// ListASTNode class declarations
class ListASTNode : public ASTNode {
public:
  ListASTNode();
  ListASTNode(vector<ASTNode *> children);
};

// FuncDeclASTNode class declarations
class FuncDeclASTNode : public ASTNode {
public:
  ListASTNode *params;
  ListASTNode *body;
  Token func_name;

  FuncDeclASTNode(Token token, vector<ASTNode *> children);

  void print(int depth = 0) override;
};

// FuncCallASTNode class declarations
class FuncCallASTNode : public ASTNode {
public:
  vector<ASTNode *> args;
  Token func_name;

  FuncCallASTNode(Token token, vector<ASTNode *> children);
};

// SetqASTNode class declarations
class SetqASTNode : public ASTNode {
public:
  Token var_name;
  ASTNode *value;

  SetqASTNode(Token token, vector<ASTNode *> children);
};

// WhileASTNode class declarations
class WhileASTNode : public ASTNode {
public:
  ASTNode *condition;
  ListASTNode *body;

  WhileASTNode(Token token, vector<ASTNode *> children);
};

// LambdaASTNode class declarations
class LambdaASTNode : public ASTNode {
public:
  ListASTNode *params;
  ListASTNode *body;

  LambdaASTNode(Token token, vector<ASTNode *> children);
};
