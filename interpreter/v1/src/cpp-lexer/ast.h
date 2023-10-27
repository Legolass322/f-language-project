#ifndef AST_H
#define AST_H

#include <graphviz/gvc.h>
#include <memory>
#include <string>
#include <vector>

using namespace std;

namespace flang {

enum TokenType {
  LITERAL,
  INT,
  REAL,
  BOOL,
  NUL,
  IDENTIFIER,
  KEYWORD,
};

struct Span {
  int line;
  int column;
};

struct Token {
  TokenType type;
  string value;

  Token();
  Token(TokenType type, string value);
};

enum ASTNodeType {
  PROGRAM,
  FUNCDEF,
  FUNCCALL,
  LAMBDA,
  LIST,
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

  virtual ~ASTNode();
  virtual void print(shared_ptr<Agraph_t> const &graph);
};

class FuncDefNode : public ASTNode {
public:
  shared_ptr<Token> name;
  shared_ptr<ASTNode> body;
  shared_ptr<ASTNode> params;

  FuncDefNode();

  FuncDefNode(shared_ptr<Token> const &head,
              vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class FuncCallNode : public ASTNode {
public:
  shared_ptr<Token> name;
  vector<shared_ptr<ASTNode>> args;

  FuncCallNode();

  FuncCallNode(shared_ptr<Token> const &head,
               vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class LambdaNode : public ASTNode {
public:
  shared_ptr<ASTNode> body;
  shared_ptr<ASTNode> params;

  LambdaNode();

  LambdaNode(shared_ptr<Token> const &head,
             vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class ListNode : public ASTNode {
public:
  ListNode();
  ListNode(vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class ReturnNode : public ASTNode {
public:
  shared_ptr<ASTNode> value;

  ReturnNode();
  ReturnNode(shared_ptr<Token> const &head,
             vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class CondNode : public ASTNode {
public:
  CondNode();
  CondNode(shared_ptr<Token> const &head,
           vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class WhileNode : public ASTNode {
public:
  shared_ptr<ASTNode> body;
  shared_ptr<ASTNode> cond;

  WhileNode();
  WhileNode(shared_ptr<Token> const &head,
            vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class ProgNode : public ASTNode {
public:
  ProgNode();
  ProgNode(shared_ptr<Token> const &head,
           vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

class SetqNode : public ASTNode {
public:
  shared_ptr<ASTNode> value;
  shared_ptr<Token> name;

  SetqNode();
  SetqNode(shared_ptr<Token> const &head,
           vector<shared_ptr<ASTNode>> const &children);

  void print(shared_ptr<Agraph_t> const &graph) override;
};

} // namespace flang

#endif // AST_H