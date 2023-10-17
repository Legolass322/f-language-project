#include "astnode.h"
#include <iostream>

using namespace std;

// ASTNode class definitions
ASTNode::ASTNode() {}

ASTNode::ASTNode(vector<ASTNode *> children) { this->children = children; }

ASTNode::ASTNode(Token token) { this->token = token; }

ASTNode::ASTNode(Token token, vector<ASTNode *> children) {
  this->token = token;
  this->children = children;
}

ASTNode::~ASTNode() {
  for (auto i : this->children) {
    delete i;
  }
}

void ASTNode::print(int depth) {
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

// ListASTNode class definitions
ListASTNode::ListASTNode() {}

ListASTNode::ListASTNode(vector<ASTNode *> children) : ASTNode(children) {}

// FuncDeclASTNode class definitions
FuncDeclASTNode::FuncDeclASTNode(Token token, vector<ASTNode *> children)
    : ASTNode(token, children) {
  this->func_name = children[0]->token;
  this->params = (ListASTNode *)children[1];
  this->body = (ListASTNode *)children[2];
}

void FuncDeclASTNode::print(int depth) {
  for (int i = 0; i < depth; i++)
    cout << "-";

  if (this->token.value != "")
    cout << this->token.value << "(" << this->func_name.value << ")" << endl;
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

// FuncCallASTNode class definitions
FuncCallASTNode::FuncCallASTNode(Token token, vector<ASTNode *> children)
    : ASTNode(token, children) {
  this->args = children;
  this->func_name = token;
}

// SetqASTNode class definitions
SetqASTNode::SetqASTNode(Token token, vector<ASTNode *> children)
    : ASTNode(token, children) {
  this->var_name = children[0]->token;
  this->value = children[1];
}

// WhileASTNode class definitions
WhileASTNode::WhileASTNode(Token token, vector<ASTNode *> children)
    : ASTNode(token, children) {
  this->condition = children[0];
  this->body = (ListASTNode *)children[1];
}

// LambdaASTNode class definitions
LambdaASTNode::LambdaASTNode(Token token, vector<ASTNode *> children)
    : ASTNode(token, children) {
  this->params = (ListASTNode *)children[0];
  this->body = (ListASTNode *)children[1];
}
