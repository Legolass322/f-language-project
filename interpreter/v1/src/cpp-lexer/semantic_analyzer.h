#include "ast.h"
#include <map>
#include <memory>

using namespace std;
using namespace flang;

struct Var {
  shared_ptr<ASTNode> value;
  int referrers;

  Var() : value(nullptr), referrers(0) {}
  Var(shared_ptr<ASTNode> value) : value(value), referrers(0) {}

  Var operator=(shared_ptr<ASTNode> value) {
    this->value = value;
    this->referrers = 0;
    return *this;
  }
};

struct Scope {
  map<string, Var> variables;
  ASTNodeType scope_type;

  Scope(map<string, Var> variables, ASTNodeType scope_type)
      : variables(variables), scope_type(scope_type) {}
};

class SemanticAnalyzer {
public:
  SemanticAnalyzer();
  ~SemanticAnalyzer();

  void analyze(shared_ptr<ASTNode> &root);

private:
  vector<Scope> scope_stack;
  map<string, shared_ptr<FuncDefNode>> functions;
  const vector<string> PF_FUNCTIONS = {
      "plus",    "minus",  "times",   "divide",    "equal",  "nonequal",
      "less",    "lesseq", "greater", "greatereq", "and",    "or",
      "not",     "xor",    "eval",    "isint",     "isreal", "isbool",
      "isnull",  "isatom", "islist",  "head",      "tail",   "cons",
      "isempty", "foldl"};

  shared_ptr<ASTNode> analyze_funcdef(shared_ptr<FuncDefNode> node);
  shared_ptr<ASTNode> analyze_funccall(shared_ptr<FuncCallNode> node);
  shared_ptr<ASTNode> analyze_lambda(shared_ptr<LambdaNode> node);
  shared_ptr<ASTNode> analyze_prog(shared_ptr<ProgNode> node);
  shared_ptr<ASTNode> analyze_node(shared_ptr<ASTNode> node);
  shared_ptr<ASTNode> analyze_setq(shared_ptr<SetqNode> node);
  shared_ptr<ASTNode> analyze_return(shared_ptr<ReturnNode> node);
  shared_ptr<ASTNode> analyze_break(shared_ptr<ASTNode> node);
  shared_ptr<ASTNode> analyze_while(shared_ptr<WhileNode> node);
  shared_ptr<ASTNode> analyze_cond(shared_ptr<CondNode> node);

  shared_ptr<ASTNode> calculate_node(shared_ptr<ASTNode> node);

  Var find_variable(shared_ptr<Token> identifier);
  shared_ptr<ASTNode> remove_variable(shared_ptr<ASTNode> node, Scope &scope,
                                      string const &identifier);
  shared_ptr<ASTNode> find_function(shared_ptr<Token> identifier);

  shared_ptr<ASTNode>
  get_inlined_function(string const &identifier,
                       vector<shared_ptr<ASTNode>> const &args);
  shared_ptr<ASTNode> inline_function(shared_ptr<ASTNode> node,
                                      map<string, string> const &args);

  shared_ptr<ASTNode> deep_copy(shared_ptr<ASTNode> node);
  shared_ptr<ASTNode> copy_node(shared_ptr<ASTNode> node,
                                vector<shared_ptr<ASTNode>> const &children);
};

class RuntimeError : public exception {
public:
  RuntimeError(Span span, string const &message) {
    this->message = "Runtime error at line " + to_string(span.line) +
                    ", column " + to_string(span.column) + ": " + message;
  }
  ~RuntimeError() throw() {}

  const char *what() const throw() { return message.c_str(); }

private:
  string message;
};

class VariableNotFoundError : public RuntimeError {
public:
  VariableNotFoundError(Span span, string const &identifier)
      : RuntimeError(span, "Variable " + identifier + " not found") {}
};

class FunctionNotFoundError : public RuntimeError {
public:
  FunctionNotFoundError(Span span, string const &identifier)
      : RuntimeError(span, "Function " + identifier + " not found") {}
};

class VariableAlreadyDefinedError : public RuntimeError {
public:
  VariableAlreadyDefinedError(Span span, string const &identifier)
      : RuntimeError(span, "Variable " + identifier + " already defined") {}
};

class FunctionAlreadyDefinedError : public RuntimeError {
public:
  FunctionAlreadyDefinedError(Span span, string const &identifier)
      : RuntimeError(span, "Function " + identifier + " already defined") {}
};

class WrongNumberOfArgumentsError : public RuntimeError {
public:
  WrongNumberOfArgumentsError(Span span, string const &identifier, int expected,
                              int got)
      : RuntimeError(span, "Function " + identifier + " expected " +
                               to_string(expected) + " arguments, got " +
                               to_string(got)) {}
};
