#include "ast.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <iostream>
#include <memory>
#include <string>

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

void ASTNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  agsafeset(graph_node.get(), (char *)"label", head->value.c_str(), (char *)"");

  for (auto child : children) {

    if (child == nullptr) {
      cerr << "child is nullptr" << endl;
      exit(1);
    }

    switch (child->node_type) {
    case FUNCDEF: {
      shared_ptr<FuncDefNode> funcdef =
          dynamic_pointer_cast<FuncDefNode>(child);
      funcdef->print(graph);
      agedge(graph.get(), graph_node.get(), funcdef->graph_node.get(), NULL,
             TRUE);
      break;
    }

    case FUNCCALL: {
      shared_ptr<FuncCallNode> funccall =
          dynamic_pointer_cast<FuncCallNode>(child);
      funccall->print(graph);
      agedge(graph.get(), graph_node.get(), funccall->graph_node.get(), NULL,
             TRUE);
      break;
    }

    case LAMBDA: {
      shared_ptr<LambdaNode> lambda = dynamic_pointer_cast<LambdaNode>(child);
      lambda->print(graph);
      agedge(graph.get(), graph_node.get(), lambda->graph_node.get(), NULL,
             TRUE);
      break;
    }

    case LIST: {
      shared_ptr<ListNode> list = dynamic_pointer_cast<ListNode>(child);
      list->print(graph);
      agedge(graph.get(), graph_node.get(), list->graph_node.get(), NULL, TRUE);
      break;
    }

    case RETURN: {
      shared_ptr<ReturnNode> ret = dynamic_pointer_cast<ReturnNode>(child);
      ret->print(graph);
      agedge(graph.get(), graph_node.get(), ret->graph_node.get(), NULL, TRUE);
      break;
    }

    case COND: {
      shared_ptr<CondNode> cond = dynamic_pointer_cast<CondNode>(child);
      cond->print(graph);
      agedge(graph.get(), graph_node.get(), cond->graph_node.get(), NULL, TRUE);
      break;
    }

    case WHILE: {
      shared_ptr<WhileNode> whilenode = dynamic_pointer_cast<WhileNode>(child);
      whilenode->print(graph);
      agedge(graph.get(), graph_node.get(), whilenode->graph_node.get(), NULL,
             TRUE);
      break;
    }

    case PROG: {
      shared_ptr<ProgNode> prog = dynamic_pointer_cast<ProgNode>(child);
      prog->print(graph);
      agedge(graph.get(), graph_node.get(), prog->graph_node.get(), NULL, TRUE);
      break;
    }

    case SETQ: {
      shared_ptr<SetqNode> setq = dynamic_pointer_cast<SetqNode>(child);
      setq->print(graph);
      agedge(graph.get(), graph_node.get(), setq->graph_node.get(), NULL, TRUE);
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

void FuncDefNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "FuncDefNode\n" + name->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  params->print(graph);
  body->print(graph);

  Agedge_t *p = agedge(graph.get(), graph_node.get(), params->graph_node.get(),
                       NULL, TRUE);
  agsafeset(p, (char *)"label", "params", (char *)"");
  Agedge_t *b =
      agedge(graph.get(), graph_node.get(), body->graph_node.get(), NULL, TRUE);
  agsafeset(b, (char *)"label", "body", (char *)"");
}

FuncCallNode::FuncCallNode() {}

FuncCallNode::FuncCallNode(shared_ptr<Token> const &head,
                           vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(FUNCCALL, head, children) {
  this->name = head;
  this->args = children;
}

void FuncCallNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "FuncCallNode\n" + name->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  for (auto arg : args) {
    arg->print(graph);
    Agedge_t *e = agedge(graph.get(), graph_node.get(), arg->graph_node.get(),
                         NULL, TRUE);
    agsafeset(e, (char *)"label", "arg", (char *)"");
  }
}

LambdaNode::LambdaNode() {}

LambdaNode::LambdaNode(shared_ptr<Token> const &head,
                       vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(LAMBDA, head, children) {
  this->params = children[0];
  this->body = children[1];
}

void LambdaNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "LambdaNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  params->print(graph);
  body->print(graph);

  Agedge_t *p = agedge(graph.get(), graph_node.get(), params->graph_node.get(),
                       NULL, TRUE);
  agsafeset(p, (char *)"label", "params", (char *)"");

  Agedge_t *b =
      agedge(graph.get(), graph_node.get(), body->graph_node.get(), NULL, TRUE);
  agsafeset(b, (char *)"label", "body", (char *)"");
}

ListNode::ListNode() {}

ListNode::ListNode(vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(LIST, nullptr, children) {
  this->children = children;
}

void ListNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "ListNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  for (auto child : children) {
    child->print(graph);
    agedge(graph.get(), graph_node.get(), child->graph_node.get(), NULL, TRUE);
  }
}

ReturnNode::ReturnNode() {}

ReturnNode::ReturnNode(shared_ptr<Token> const &head,
                       vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(RETURN, head, children) {
  this->value = children[0];
}

void ReturnNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "ReturnNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  value->print(graph);

  Agedge_t *e = agedge(graph.get(), graph_node.get(), value->graph_node.get(),
                       NULL, TRUE);
  agsafeset(e, (char *)"label", "value", (char *)"");
}

CondNode::CondNode() {}

CondNode::CondNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(COND, head, children) {}

void CondNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "CondNode" << endl;
}

WhileNode::WhileNode() {}

WhileNode::WhileNode(shared_ptr<Token> const &head,
                     vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(WHILE, head, children) {
  this->cond = children[0];
  this->body = children[1];
}

void WhileNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "WhileNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  cond->print(graph);
  body->print(graph);

  Agedge_t *c =
      agedge(graph.get(), graph_node.get(), cond->graph_node.get(), NULL, TRUE);
  agsafeset(c, (char *)"label", "cond", (char *)"");
  Agedge_t *b =
      agedge(graph.get(), graph_node.get(), body->graph_node.get(), NULL, TRUE);
  agsafeset(b, (char *)"label", "body", (char *)"");
}

ProgNode::ProgNode() {}

ProgNode::ProgNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(PROG, head, children) {}

void ProgNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "ProgNode" << endl;
  // TODO: print
}

SetqNode::SetqNode() {}

SetqNode::SetqNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(SETQ, head, children) {
  this->name = children[0]->head;
  this->value = children[1];
}

void SetqNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "SetqNode\n" + name->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  value->print(graph);

  Agedge_t *e = agedge(graph.get(), graph_node.get(), value->graph_node.get(),
                       NULL, TRUE);
  agsafeset(e, (char *)"label", "value", (char *)"");
}