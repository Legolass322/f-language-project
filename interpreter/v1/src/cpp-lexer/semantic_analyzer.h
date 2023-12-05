#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "driver.hh"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>

using namespace flang;
using std::make_shared, std::map, std::shared_ptr, std::string, std::to_string,
    std::vector;

struct Var {
  shared_ptr<ASTNode> value;
  int referrers;

  Var() : value(nullptr), referrers(0) {}
  Var(shared_ptr<ASTNode> value) : value(value), referrers(0) {}
  Var(shared_ptr<ASTNode> value, int referrers)
      : value(value), referrers(referrers) {}

  Var operator=(shared_ptr<ASTNode> value) {
    this->value = value;
    this->referrers = 0;
    return *this;
  }
};

struct Scope {
  map<string, Var> variables;
  shared_ptr<ASTNode> scope_node;

  Scope(shared_ptr<ASTNode> scope_node)
      : variables(map<string, Var>()), scope_node(scope_node) {}
};

class SemanticAnalyzer {
public:
  SemanticAnalyzer();
  ~SemanticAnalyzer();

  void analyze(shared_ptr<ASTNode> &root);
  void clear_stack(shared_ptr<ASTNode> &root);

private:
  vector<Scope> scope_stack;
  int tmp_counter;
  const vector<string> PF_FUNCTIONS = {
      "plus",    "minus",   "times",      "divide",    "equal",  "nonequal",
      "less",    "lesseq",  "greater",    "greatereq", "and",    "or",
      "not",     "xor",     "eval",       "isint",     "isreal", "isbool",
      "isnull",  "isatom",  "islist",     "head",      "tail",   "cons",
      "isempty", "println", "_trampoline"};

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
  get_inlined_function(shared_ptr<FuncDefNode> const &funcdef,
                       vector<shared_ptr<ASTNode>> const &args);

  void mark_inlined_function(shared_ptr<Token> const &identifier);

  shared_ptr<ASTNode>
  is_recursive_call(shared_ptr<FuncDefNode> const &funcdef,
                    vector<shared_ptr<ASTNode>> const &args);

  shared_ptr<ASTNode>
  inline_function(shared_ptr<ASTNode> node, map<string, string> const &tmps,
                  map<string, shared_ptr<ASTNode>> const &params_to_args);

  shared_ptr<ASTNode> inline_require(shared_ptr<ASTNode> node);
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

#endif
