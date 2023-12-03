#ifndef PF_FUNCS_H
#define PF_FUNCS_H

#include "ast.h"
#include "semantic_analyzer.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

using namespace flang;

shared_ptr<ASTNode> pf_plus(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_minus(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_times(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_divide(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_equal(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_nonequal(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_less(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_lesseq(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_greater(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_greatereq(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_and(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_or(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_not(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_xor(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_eval(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_isint(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_isreal(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_isbool(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_isnull(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_isatom(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_islist(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_head(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_tail(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_cons(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_isempty(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_foldl(vector<shared_ptr<ASTNode>> &args);
shared_ptr<ASTNode> pf_println(vector<shared_ptr<ASTNode>> &args);

void print_func(shared_ptr<ASTNode> const &node);
void print_string(shared_ptr<ASTNode> const &node);
bool is_string(shared_ptr<ASTNode> const &node);

#endif
