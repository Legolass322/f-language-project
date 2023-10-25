#include "ast.h"

#include <iostream>
#include <memory>

using namespace flang;

Token::Token() {}

Token::Token(TokenType type, string value) : type(type), value(value) {}

ASTNode::ASTNode(ASTNodeType node_type, shared_ptr<Token> const &head)
    : node_type(node_type), head(head) {
  this->head = head;
}

ASTNode::ASTNode(ASTNodeType node_type, shared_ptr<Token> const &head,
                 vector<shared_ptr<ASTNode>> const &children)
    : node_type(node_type), head(head), children(children) {
  this->head = head;
  this->children = children;
}

ASTNode::ASTNode() {}

ASTNode::~ASTNode() {}

void ASTNode::print() {
  cout << "ASTNode" << endl;
  cout << "head: " << endl;
  cout << "type: " << head->type << endl;
  cout << "value: " << head->value << endl;
  cout << "children: " << endl;
  for (auto child : children) {
    if (child == nullptr) {
      cout << "nullptr" << endl;
      continue;
    }
    switch (child->node_type) {
    case FUNCDEF: {
      shared_ptr<FuncDefNode> funcdef =
          dynamic_pointer_cast<FuncDefNode>(child);
      funcdef->print();
      break;
    }

    case FUNCCALL: {
      shared_ptr<FuncCallNode> funccall =
          dynamic_pointer_cast<FuncCallNode>(child);
      funccall->print();
      break;
    }

    case LAMBDA: {
      shared_ptr<LambdaNode> lambda = dynamic_pointer_cast<LambdaNode>(child);
      lambda->print();
      break;
    }

    case LIST: {
      shared_ptr<ListNode> list = dynamic_pointer_cast<ListNode>(child);
      list->print();
      break;
    }

    case RETURN: {
      shared_ptr<ReturnNode> ret = dynamic_pointer_cast<ReturnNode>(child);
      ret->print();
      break;
    }

    case COND: {
      shared_ptr<CondNode> cond = dynamic_pointer_cast<CondNode>(child);
      cond->print();
      break;
    }

    case WHILE: {
      shared_ptr<WhileNode> whilenode = dynamic_pointer_cast<WhileNode>(child);
      whilenode->print();
      break;
    }

    case PROG: {
      shared_ptr<ProgNode> prog = dynamic_pointer_cast<ProgNode>(child);
      prog->print();
      break;
    }

    case SETQ: {
      shared_ptr<SetqNode> setq = dynamic_pointer_cast<SetqNode>(child);
      setq->print();
      break;
    }

    default: {
      cout << "Unknown node type" << endl;
      break;
    }
    }
  }
}

FuncDefNode::FuncDefNode() {}

FuncDefNode::FuncDefNode(shared_ptr<Token> const &head,
                         vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(FUNCDEF, head, children) {
  this->name = children[0]->head;
  this->params = children[1];
  this->body = children[2];
}

void FuncDefNode::print() {
  cout << "FuncDefNode" << endl;
  cout << "name: " << name->value << endl;
  cout << "params: " << endl;
  params->print();
  cout << "body: " << endl;
  body->print();
}

FuncCallNode::FuncCallNode() {}

FuncCallNode::FuncCallNode(shared_ptr<Token> const &head,
                           vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(FUNCCALL, head, children) {
  this->name = children[0]->head;
  this->args = children[1];
}

void FuncCallNode::print() {
  cout << "FuncCallNode" << endl;
  cout << "name: " << name->value << endl;
  cout << "args: " << endl;
  args->print();
}

LambdaNode::LambdaNode() {}

LambdaNode::LambdaNode(shared_ptr<Token> const &head,
                       vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(LAMBDA, head, children) {
  this->params = children[0];
  this->body = children[1];
}

void LambdaNode::print() {
  cout << "LambdaNode" << endl;
  cout << "params: " << endl;
  params->print();
  cout << "body: " << endl;
  body->print();
}

ListNode::ListNode() {}

ListNode::ListNode(vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(LIST, nullptr, children) {
  this->children = children;
}

void ListNode::print() {
  cout << "ListNode" << endl;
  for (auto child : children) {
    child->print();
  }
}

ReturnNode::ReturnNode() {}

ReturnNode::ReturnNode(shared_ptr<Token> const &head,
                       vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(RETURN, head, children) {
  this->value = children[0];
}

void ReturnNode::print() {
  cout << "ReturnNode" << endl;
  cout << "value: " << endl;
  value->print();
}

CondNode::CondNode() {}

CondNode::CondNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(COND, head, children) {}

void CondNode::print() { cout << "CondNode" << endl; }

WhileNode::WhileNode() {}

WhileNode::WhileNode(shared_ptr<Token> const &head,
                     vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(WHILE, head, children) {
  this->cond = children[0];
  this->body = children[1];
}

void WhileNode::print() {
  cout << "WhileNode" << endl;
  cout << "cond: " << endl;
  cond->print();
  cout << "body: " << endl;
  body->print();
}

ProgNode::ProgNode() {}

ProgNode::ProgNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(PROG, head, children) {}

void ProgNode::print() { cout << "ProgNode" << endl; }

SetqNode::SetqNode() {}

SetqNode::SetqNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(SETQ, head, children) {
  this->name = children[0]->head;
  this->value = children[1];
}

void SetqNode::print() {
  cout << "SetqNode" << endl;
  cout << "name: " << name->value << endl;
  cout << "value: " << endl;
  value->print();
}
