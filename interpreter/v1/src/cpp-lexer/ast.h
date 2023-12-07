#ifndef AST_H
#define AST_H

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

namespace flang {

enum TokenType { LITERAL, INT, REAL, BOOL, NUL, IDENTIFIER, KEYWORD, CHAR };

struct Span {
  int line;
  int column;
};

struct Token {
  TokenType type;
  string value;
  Span span;

  Token();
  Token(TokenType type, string value, Span span);
};

enum ASTNodeType {
  PROGRAM,
  FUNCDEF,
  FUNCCALL,
  LAMBDA,
  LIST,
  QUOTE_LIST,
  RETURN,
  BREAK,
  COND,
  WHILE,
  PROG,
  SETQ,
  LEAF,
};

class ASTNode {
public:
  shared_ptr<Agnode_t> graph_node;
  shared_ptr<Token> head;
  vector<shared_ptr<ASTNode>> children;
  ASTNodeType node_type;

  ASTNode();

  ASTNode(ASTNodeType node_type, shared_ptr<Token> const &head);

  ASTNode(ASTNodeType node_type, shared_ptr<Token> const &head,
          vector<shared_ptr<ASTNode>> const &children);

  bool calculable();

  virtual ~ASTNode();
  virtual void print(shared_ptr<Agraph_t> const &graph);
  virtual shared_ptr<ASTNode> copy();
};

class FuncDefNode : public ASTNode {
public:
  bool is_recursive;
  bool is_inlined = false;
  bool is_tail_recursive;
  shared_ptr<Token> getName();
  shared_ptr<ASTNode> getBody();
  shared_ptr<ASTNode> getParams();

  void setBody(shared_ptr<ASTNode> const &body);
  void setParams(shared_ptr<ASTNode> const &params);
  void setName(shared_ptr<Token> const &name);

  FuncDefNode();

  FuncDefNode(shared_ptr<Token> const &head,
              vector<shared_ptr<ASTNode>> const &children,
              bool is_recursive = false, bool is_tail_recursive = false);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class FuncCallNode : public ASTNode {
public:
  shared_ptr<Token> getName();
  vector<shared_ptr<ASTNode>> getArgs();

  void setArgs(vector<shared_ptr<ASTNode>> const &args);
  void setName(shared_ptr<Token> const &name);

  FuncCallNode();

  FuncCallNode(shared_ptr<Token> const &head,
               vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class LambdaNode : public ASTNode {
public:
  shared_ptr<ASTNode> getBody();
  shared_ptr<ASTNode> getParams();

  void setBody(shared_ptr<ASTNode> const &body);
  void setParams(shared_ptr<ASTNode> const &params);

  LambdaNode();

  LambdaNode(shared_ptr<Token> const &head,
             vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class ListNode : public ASTNode {
public:
  ListNode();
  ListNode(vector<shared_ptr<ASTNode>> const &children);
  ListNode(bool is_quote, vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class ReturnNode : public ASTNode {
public:
  shared_ptr<ASTNode> getValue();

  void setValue(shared_ptr<ASTNode> const &value);

  ReturnNode();
  ReturnNode(shared_ptr<Token> const &head,
             vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class CondNode : public ASTNode {
public:
  shared_ptr<ASTNode> getCond();
  shared_ptr<ASTNode> getBranchTrue();
  shared_ptr<ASTNode> getBranchFalse();

  void setCond(shared_ptr<ASTNode> const &cond);
  void setBranchTrue(shared_ptr<ASTNode> const &branch_true);
  void setBranchFalse(shared_ptr<ASTNode> const &branch_false);

  CondNode();
  CondNode(shared_ptr<Token> const &head,
           vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class WhileNode : public ASTNode {
public:
  shared_ptr<ASTNode> getBody();
  shared_ptr<ASTNode> getCond();

  void setBody(shared_ptr<ASTNode> const &body);
  void setCond(shared_ptr<ASTNode> const &cond);

  WhileNode();
  WhileNode(shared_ptr<Token> const &head,
            vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class ProgNode : public ASTNode {
public:
  bool is_inlined;
  shared_ptr<ASTNode> getLocals();

  void setLocals(shared_ptr<ASTNode> const &locals);

  ProgNode();
  ProgNode(shared_ptr<Token> const &head,
           vector<shared_ptr<ASTNode>> const &children,
           bool is_inlined = false);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

class SetqNode : public ASTNode {
public:
  shared_ptr<ASTNode> getValue();
  shared_ptr<Token> getName();

  void setValue(shared_ptr<ASTNode> const &value);
  void setName(shared_ptr<Token> const &name);

  SetqNode();
  SetqNode(shared_ptr<Token> const &head,
           vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
  shared_ptr<ASTNode> copy() override;
};

shared_ptr<Token> calculate(vector<shared_ptr<ASTNode>> const &args,
                            string const &op);

// overload cout

wostream &operator<<(wostream &os, const Token &token);
wostream &operator<<(wostream &os, const ASTNode &node);
wostream &operator<<(wostream &os, const FuncDefNode &node);
wostream &operator<<(wostream &os, const FuncCallNode &node);
wostream &operator<<(wostream &os, const LambdaNode &node);
wostream &operator<<(wostream &os, const ListNode &node);
wostream &operator<<(wostream &os, const ReturnNode &node);
wostream &operator<<(wostream &os, const CondNode &node);
wostream &operator<<(wostream &os, const WhileNode &node);
wostream &operator<<(wostream &os, const ProgNode &node);
wostream &operator<<(wostream &os, const SetqNode &node);

} // namespace flang

#endif // AST_H
