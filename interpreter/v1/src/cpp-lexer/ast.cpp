#include "ast.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <iostream>
#include <memory>
#include <string>

using namespace flang;

Token::Token() : type(NUL), value(""), span(Span({0, 0})) {}

Token::Token(TokenType type, string value, Span span)
    : type(type), value(value), span(span) {}

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

void ASTNode::update() { this->children = children; }

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

    case LEAF: {
      child->print(graph);
      agedge(graph.get(), graph_node.get(), child->graph_node.get(), NULL,
             TRUE);
      break;
    }

    case PROGRAM: {
      child->print(graph);
      agedge(graph.get(), graph_node.get(), child->graph_node.get(), NULL,
             TRUE);
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

void FuncDefNode::update() {
  this->params = children[1];
  this->body = children[2];
}

void FuncDefNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print funcdef " << name->value << endl;
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

void FuncCallNode::update() { this->args = children; }

void FuncCallNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print funccall " << name->value << endl;
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

void LambdaNode::update() {
  this->params = children[0];
  this->body = children[1];
}

void LambdaNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print lambda" << endl;
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
    : ASTNode(LIST, make_shared<Token>(), children) {
  this->children = children;
}

ListNode::ListNode(bool is_quote, vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(is_quote ? QUOTE_LIST : LIST, make_shared<Token>(), children) {
  this->children = children;
}

void ListNode::update() { this->children = children; }

void ListNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print list" << endl;
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

void ReturnNode::update() { this->value = children[0]; }

void ReturnNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print return" << endl;
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "ReturnNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  value->print(graph);

  Agedge_t *e = agedge(graph.get(), graph_node.get(), value->graph_node.get(),
                       NULL, TRUE);
  agsafeset(e, (char *)"label", "value", (char *)"");
  cout << "print return end" << endl;
}

CondNode::CondNode() {}

CondNode::CondNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(COND, head, children) {
  this->cond = children[0];
  this->body_true = children[1];

  if (children.size() == 3) {
    this->body_false = children[2];
  }
}

void CondNode::update() {
  this->cond = children[0];
  this->body_true = children[1];

  if (children.size() == 3) {
    this->body_false = children[2];
  }
}

void CondNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print cond" << endl;
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "CondNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  cond->print(graph);
  body_true->print(graph);

  Agedge_t *c =
      agedge(graph.get(), graph_node.get(), cond->graph_node.get(), NULL, TRUE);
  agsafeset(c, (char *)"label", "cond", (char *)"");
  Agedge_t *t = agedge(graph.get(), graph_node.get(),
                       body_true->graph_node.get(), NULL, TRUE);
  agsafeset(t, (char *)"label", "body_true", (char *)"");

  if (body_false != nullptr) {
    body_false->print(graph);
    Agedge_t *f = agedge(graph.get(), graph_node.get(),
                         body_false->graph_node.get(), NULL, TRUE);
    agsafeset(f, (char *)"label", "body_false", (char *)"");
  }
}

WhileNode::WhileNode() {}

WhileNode::WhileNode(shared_ptr<Token> const &head,
                     vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(WHILE, head, children) {
  this->cond = children[0];
  this->body = children[1];
}

void WhileNode::update() {
  this->cond = children[0];
  this->body = children[1];
}

void WhileNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print while" << endl;
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
    : ASTNode(PROG, head, children) {
  this->locals = children[0];
}

void ProgNode::update() { this->locals = children[0]; }

void ProgNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print prog" << endl;
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "ProgNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  locals->print(graph);

  Agedge_t *l = agedge(graph.get(), graph_node.get(), locals->graph_node.get(),
                       NULL, TRUE);
  agsafeset(l, (char *)"label", "locals", (char *)"");

  for (int i = 1; i < children.size(); i++) {
    children[i]->print(graph);
    Agedge_t *e = agedge(graph.get(), graph_node.get(),
                         children[i]->graph_node.get(), NULL, TRUE);
  }
}

SetqNode::SetqNode() {}

SetqNode::SetqNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(SETQ, head, children) {
  this->name = children[0]->head;
  this->value = children[1];
}

void SetqNode::update() {
  this->name = children[0]->head;
  this->value = children[1];
}

void SetqNode::print(shared_ptr<Agraph_t> const &graph) {
  cout << "print setq" << endl;
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "SetqNode\n" + name->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  value->print(graph);

  Agedge_t *e = agedge(graph.get(), graph_node.get(), value->graph_node.get(),
                       NULL, TRUE);
  agsafeset(e, (char *)"label", "value", (char *)"");
}

bool flang::calculable(vector<shared_ptr<ASTNode>> const &args) {

  for (auto arg : args) {
    if (arg->node_type == LEAF && arg->head->type != NUL &&
        arg->head->type != IDENTIFIER) {
      continue;
    } else
      return false;
  }

  return true;
}

bool flang::calculable(const shared_ptr<ASTNode> &node) {
  return node->node_type == LEAF && node->head->type != NUL &&
         node->head->type != IDENTIFIER;
}

shared_ptr<Token> flang::calculate(vector<shared_ptr<ASTNode>> const &args,
                                   string const &op) {
  double result;

  if (op == "plus") {
    result = 0.0;
    for (auto arg : args) {
      double arg_val = stod(arg->head->value);
      result += arg_val;
    }
  } else if (op == "minus") {
    double arg_val = stod(args[0]->head->value);
    result = arg_val;
    for (int i = 1; i < args.size(); i++) {
      arg_val = stod(args[i]->head->value);
      result -= arg_val;
    }
  } else if (op == "times") {
    result = 1.0;
    for (auto arg : args) {
      double arg_val = stod(arg->head->value);
      result *= arg_val;
    }
  } else if (op == "divide") {
    double arg_val = stod(args[0]->head->value);
    result = arg_val;
    for (int i = 1; i < args.size(); i++) {
      arg_val = stod(args[i]->head->value);
      result /= arg_val;
    }
  } else {
    std::cerr << "Unknown operator" << std::endl;
    exit(1);
  }

  double intpart;
  if (modf(result, &intpart) == 0.0) {
    return make_shared<Token>(INT, to_string((int)result), args[0]->head->span);
  } else {
    return make_shared<Token>(REAL, to_string(result), args[0]->head->span);
  }
}
