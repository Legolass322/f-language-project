#include "ast.h"

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

ASTNode::~ASTNode() {
  for (auto child : children) {
    child.reset();
  }
  children.clear();
}

shared_ptr<ASTNode> ASTNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<ASTNode>(node_type, head, children_copy);
}

bool ASTNode::calculable() {
  return this->node_type == LEAF && this->head->type != NUL &&
         this->head->type != IDENTIFIER;
}

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
                         vector<shared_ptr<ASTNode>> const &children,
                         bool is_recursive)
    : ASTNode(FUNCDEF, head, children) {
  this->is_recursive = is_recursive;
}

shared_ptr<Token> FuncDefNode::getName() { return children[0]->head; }
shared_ptr<ASTNode> FuncDefNode::getBody() { return children[2]; }
shared_ptr<ASTNode> FuncDefNode::getParams() { return children[1]; }

void FuncDefNode::setName(shared_ptr<Token> const &name) {
  children[0]->head = name;
}

void FuncDefNode::setBody(shared_ptr<ASTNode> const &body) {
  children[2] = body;
}

void FuncDefNode::setParams(shared_ptr<ASTNode> const &params) {
  children[1] = params;
}

shared_ptr<ASTNode> FuncDefNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<FuncDefNode>(head, children_copy, is_recursive);
}

void FuncDefNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "FuncDefNode\n" + getName()->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  getParams()->print(graph);
  getBody()->print(graph);

  Agedge_t *p = agedge(graph.get(), graph_node.get(),
                       getParams()->graph_node.get(), NULL, TRUE);
  agsafeset(p, (char *)"label", "params", (char *)"");
  Agedge_t *b = agedge(graph.get(), graph_node.get(),
                       getBody()->graph_node.get(), NULL, TRUE);
  agsafeset(b, (char *)"label", "body", (char *)"");
}

FuncCallNode::FuncCallNode() {}

FuncCallNode::FuncCallNode(shared_ptr<Token> const &head,
                           vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(FUNCCALL, head, children) {}

shared_ptr<Token> FuncCallNode::getName() {
  if (children[0]->node_type != LEAF)
    return head;

  head = children[0]->head;

  return children[0]->head;
}
vector<shared_ptr<ASTNode>> FuncCallNode::getArgs() {
  return children.size() > 1
             ? vector<shared_ptr<ASTNode>>(children.begin() + 1, children.end())
             : vector<shared_ptr<ASTNode>>();
}

void FuncCallNode::setName(shared_ptr<Token> const &name) { head = name; }
void FuncCallNode::setArgs(vector<shared_ptr<ASTNode>> const &args) {
  for (int i = 0; i < args.size(); i++) {
    if (i + 1 >= children.size())
      children.push_back(args[i]);
    else
      children[i + 1] = args[i];
  }
}

shared_ptr<ASTNode> FuncCallNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<FuncCallNode>(head, children_copy);
}

void FuncCallNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "FuncCallNode\n" + getName()->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  for (auto arg : getArgs()) {
    arg->print(graph);
    Agedge_t *e = agedge(graph.get(), graph_node.get(), arg->graph_node.get(),
                         NULL, TRUE);
    agsafeset(e, (char *)"label", "arg", (char *)"");
  }
}

LambdaNode::LambdaNode() {}

LambdaNode::LambdaNode(shared_ptr<Token> const &head,
                       vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(LAMBDA, head, children) {}

shared_ptr<ASTNode> LambdaNode::getBody() { return children[1]; }
shared_ptr<ASTNode> LambdaNode::getParams() { return children[0]; }

void LambdaNode::setBody(shared_ptr<ASTNode> const &body) {
  children[1] = body;
}

void LambdaNode::setParams(shared_ptr<ASTNode> const &params) {
  children[0] = params;
}

shared_ptr<ASTNode> LambdaNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<LambdaNode>(head, children_copy);
}

void LambdaNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "LambdaNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  getParams()->print(graph);
  getBody()->print(graph);

  Agedge_t *p = agedge(graph.get(), graph_node.get(),
                       getParams()->graph_node.get(), NULL, TRUE);
  agsafeset(p, (char *)"label", "params", (char *)"");

  Agedge_t *b = agedge(graph.get(), graph_node.get(),
                       getBody()->graph_node.get(), NULL, TRUE);
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

shared_ptr<ASTNode> ListNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<ListNode>(children_copy);
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
    : ASTNode(RETURN, head, children) {}

shared_ptr<ASTNode> ReturnNode::getValue() { return children[0]; }

void ReturnNode::setValue(shared_ptr<ASTNode> const &value) {
  children[0] = value;
}

shared_ptr<ASTNode> ReturnNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<ReturnNode>(head, children_copy);
}

void ReturnNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "ReturnNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  getValue()->print(graph);

  Agedge_t *e = agedge(graph.get(), graph_node.get(),
                       getValue()->graph_node.get(), NULL, TRUE);
  agsafeset(e, (char *)"label", "value", (char *)"");
}

CondNode::CondNode() {}

CondNode::CondNode(shared_ptr<Token> const &head,
                   vector<shared_ptr<ASTNode>> const &children)
    : ASTNode(COND, head, children) {}

shared_ptr<ASTNode> CondNode::getCond() { return children[0]; }
shared_ptr<ASTNode> CondNode::getBranchTrue() { return children[1]; }
shared_ptr<ASTNode> CondNode::getBranchFalse() {
  if (children.size() == 3)
    return children[2];
  else
    return nullptr;
}

shared_ptr<ASTNode> CondNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<CondNode>(head, children_copy);
}

void CondNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "CondNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  shared_ptr<ASTNode> cond = getCond();
  shared_ptr<ASTNode> body_true = getBranchTrue();
  shared_ptr<ASTNode> body_false = getBranchFalse();

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
    : ASTNode(WHILE, head, children) {}

shared_ptr<ASTNode> WhileNode::getBody() { return children[1]; }
shared_ptr<ASTNode> WhileNode::getCond() { return children[0]; }

void WhileNode::setBody(shared_ptr<ASTNode> const &body) { children[1] = body; }
void WhileNode::setCond(shared_ptr<ASTNode> const &cond) { children[0] = cond; }

shared_ptr<ASTNode> WhileNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<WhileNode>(head, children_copy);
}

void WhileNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "WhileNode\n";
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  shared_ptr<ASTNode> cond = getCond();
  shared_ptr<ASTNode> body = getBody();

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
                   vector<shared_ptr<ASTNode>> const &children, bool is_inlined)
    : ASTNode(PROG, head, children) {
  this->is_inlined = is_inlined;
}

shared_ptr<ASTNode> ProgNode::getLocals() { return children[0]; }

shared_ptr<ASTNode> ProgNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<ProgNode>(head, children_copy, is_inlined);
}

void ProgNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string inlined = is_inlined ? "inlined" : "";
  string label = "ProgNode\n" + inlined;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  shared_ptr<ASTNode> locals = getLocals();

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
    : ASTNode(SETQ, head, children) {}

shared_ptr<Token> SetqNode::getName() { return children[0]->head; }
shared_ptr<ASTNode> SetqNode::getValue() { return children[1]; }

void SetqNode::setName(shared_ptr<Token> const &name) {
  children[0]->head = name;
}

void SetqNode::setValue(shared_ptr<ASTNode> const &value) {
  children[1] = value;
}

shared_ptr<ASTNode> SetqNode::copy() {
  vector<shared_ptr<ASTNode>> children_copy;
  for (auto child : children) {
    children_copy.push_back(child->copy());
  }

  return make_shared<SetqNode>(head, children_copy);
}

void SetqNode::print(shared_ptr<Agraph_t> const &graph) {
  this->graph_node = shared_ptr<Agnode_t>(agnode(graph.get(), NULL, TRUE));
  string label = "SetqNode\n" + getName()->value;
  agsafeset(graph_node.get(), (char *)"label", label.c_str(), (char *)"");

  getValue()->print(graph);

  Agedge_t *e = agedge(graph.get(), graph_node.get(),
                       getValue()->graph_node.get(), NULL, TRUE);
  agsafeset(e, (char *)"label", "value", (char *)"");
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

ostream &flang::operator<<(ostream &os, const Token &token) {
  os << token.value;
  return os;
}

ostream &flang::operator<<(ostream &os, const ASTNode &node) {
  if (node.node_type == BREAK) {
    os << "<break>";
    return os;
  }

  os << node.head->value;
  return os;
}

ostream &flang::operator<<(ostream &os, const FuncDefNode &node) {
  os << "<function " << node.children[0]->head->value << ">";
  return os;
}

ostream &flang::operator<<(ostream &os, const FuncCallNode &node) {
  os << "<function call " << node.children[0]->head->value << ">";
  return os;
}

ostream &flang::operator<<(ostream &os, const LambdaNode &node) {
  os << "<lambda>";
  return os;
}

ostream &flang::operator<<(ostream &os, const ListNode &node) {
  os << "(";
  for (int i = 0; i < node.children.size(); i++) {
    os << *node.children[i];
    if (i != node.children.size() - 1)
      os << " ";
  }
  os << ")";
  return os;
}

ostream &flang::operator<<(ostream &os, const ReturnNode &node) {
  os << "<return>";
  return os;
}

ostream &flang::operator<<(ostream &os, const CondNode &node) {
  os << "<cond>";
  return os;
}

ostream &flang::operator<<(ostream &os, const WhileNode &node) {
  os << "<while>";
  return os;
}

ostream &flang::operator<<(ostream &os, const ProgNode &node) {
  os << "<prog>";
  return os;
}

ostream &flang::operator<<(ostream &os, const SetqNode &node) {
  os << "<setq>";
  return os;
}
