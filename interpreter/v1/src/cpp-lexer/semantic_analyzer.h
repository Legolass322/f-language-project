#include "ast.h"
#include <map>
#include <memory>

using namespace std;

struct Scope {
  map<string, shared_ptr<flang::ASTNode>> variables;
  flang::ASTNodeType scope_type;

  Scope(map<string, shared_ptr<flang::ASTNode>> variables,
        flang::ASTNodeType scope_type)
      : variables(variables), scope_type(scope_type) {}
};

class SemanticAnalyzer {
public:
  SemanticAnalyzer();
  ~SemanticAnalyzer();

  void analyze(shared_ptr<flang::ASTNode> root);

private:
  vector<Scope> scope_stack;
  map<string, shared_ptr<flang::FuncDefNode>> functions;
  const vector<string> PF_FUNCTIONS = {
      "plus",   "minus",  "times",   "divide",    "equal",  "nonequal",
      "less",   "lesseq", "greater", "greatereq", "and",    "or",
      "not",    "xor",    "eval",    "isint",     "isreal", "isbool",
      "isnull", "isatom", "islist",  "head",      "tail",   "cons"};

  void analyze_funcdef(shared_ptr<flang::FuncDefNode> node);
  void analyze_funccall(shared_ptr<flang::FuncCallNode> node);
  void analyze_lambda(shared_ptr<flang::LambdaNode> node);
  void analyze_prog(shared_ptr<flang::ProgNode> node);
  void analyze_node(shared_ptr<flang::ASTNode> node);
  void analyze_setq(shared_ptr<flang::SetqNode> node);
  void analyze_return(shared_ptr<flang::ReturnNode> node);
  void analyze_break(shared_ptr<flang::ASTNode> node);
  void analyze_while(shared_ptr<flang::WhileNode> node);
  void analyze_cond(shared_ptr<flang::CondNode> node);

  void calculate_node(shared_ptr<flang::ASTNode> node);

  shared_ptr<flang::ASTNode> find_variable(shared_ptr<flang::Token> identifier);
  shared_ptr<flang::ASTNode> find_function(shared_ptr<flang::Token> identifier);
};

class RuntimeError : public exception {
public:
  RuntimeError(flang::Span span, string const &message) {
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
  VariableNotFoundError(flang::Span span, string const &identifier)
      : RuntimeError(span, "Variable " + identifier + " not found") {}
};

class FunctionNotFoundError : public RuntimeError {
public:
  FunctionNotFoundError(flang::Span span, string const &identifier)
      : RuntimeError(span, "Function " + identifier + " not found") {}
};

class VariableAlreadyDefinedError : public RuntimeError {
public:
  VariableAlreadyDefinedError(flang::Span span, string const &identifier)
      : RuntimeError(span, "Variable " + identifier + " already defined") {}
};

class FunctionAlreadyDefinedError : public RuntimeError {
public:
  FunctionAlreadyDefinedError(flang::Span span, string const &identifier)
      : RuntimeError(span, "Function " + identifier + " already defined") {}
};
