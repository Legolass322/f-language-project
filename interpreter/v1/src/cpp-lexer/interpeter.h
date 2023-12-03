#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "pf_funcs.h"
#include "utils.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>

using namespace flang;

namespace interp {

struct Scope {
  map<string, shared_ptr<ASTNode>> variables;
  shared_ptr<ASTNode> return_value;
  ASTNodeType scope_type;
  bool break_flag;
  bool inlined;

  Scope();
  Scope(ASTNodeType scope_type, bool inlined = false);

  shared_ptr<ASTNode> &operator[](string const &key);
};

class Interpreter {
public:
  Interpreter();
  ~Interpreter();
  shared_ptr<ASTNode> interpret(shared_ptr<ASTNode> const &node);

private:
  const vector<string> PF_FUNCS = {
      "plus",    "minus",  "times",   "divide",    "equal",  "nonequal",
      "less",    "lesseq", "greater", "greatereq", "and",    "or",
      "not",     "xor",    "eval",    "isint",     "isreal", "isbool",
      "isnull",  "isatom", "islist",  "head",      "tail",   "cons",
      "isempty", "foldl",  "println", "require"};

  const map<string, shared_ptr<ASTNode> (*)(vector<shared_ptr<ASTNode>> &)>
      PF_FUNC_MAP = {{"plus", pf_plus},
                     {"minus", pf_minus},
                     {"times", pf_times},
                     {"divide", pf_divide},
                     {"println", pf_println},
                     {"equal", pf_equal},
                     {"nonequal", pf_nonequal},
                     {"less", pf_less},
                     {"lesseq", pf_lesseq},
                     {"greater", pf_greater},
                     {"greatereq", pf_greatereq},
                     {"and", pf_and},
                     {"or", pf_or},
                     {"not", pf_not},
                     {"xor", pf_xor},
                     {"eval", pf_eval},
                     {"isint", pf_isint},
                     {"isreal", pf_isreal},
                     {"isbool", pf_isbool},
                     {"isnull", pf_isnull},
                     {"isatom", pf_isatom},
                     {"islist", pf_islist},
                     {"head", pf_head},
                     {"tail", pf_tail},
                     {"cons", pf_cons},
                     {"isempty", pf_isempty}};

  vector<Scope> stack;

  void interpret_program(shared_ptr<ASTNode> const &node);
  void interpret_setq(shared_ptr<SetqNode> const &node);
  void interpret_break(shared_ptr<ASTNode> const &node);
  void interpret_while(shared_ptr<WhileNode> const &node);

  shared_ptr<ASTNode> interpret_lambda(shared_ptr<LambdaNode> const &node);
  shared_ptr<ASTNode> interpret_funccall(shared_ptr<FuncCallNode> const &node);
  shared_ptr<ASTNode> interpret_list(shared_ptr<ListNode> const &node);
  shared_ptr<ASTNode> interpret_return(shared_ptr<ReturnNode> const &node);
  shared_ptr<ASTNode> interpret_cond(shared_ptr<CondNode> const &node);
  shared_ptr<ASTNode> interpret_prog(shared_ptr<ProgNode> const &node);
  shared_ptr<ASTNode> interpret_leaf(shared_ptr<ASTNode> const &node);
  shared_ptr<ASTNode> interpret_funcdef(shared_ptr<FuncDefNode> const &node);

  shared_ptr<ASTNode>
  interpret_func_closure(shared_ptr<FuncDefNode> const &node);
  shared_ptr<ASTNode>
  interpret_lambda_closure(shared_ptr<LambdaNode> const &node);
  void iterate_closure(shared_ptr<ASTNode> const &node,
                       vector<shared_ptr<ASTNode>> &setqs,
                       vector<string> const &defined);

  void eval_result(shared_ptr<ASTNode> const &node, bool is_recursive = false);
};

} // namespace interp

#endif
