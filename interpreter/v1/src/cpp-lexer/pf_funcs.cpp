#include "pf_funcs.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include <cmath>
#include <iostream>

shared_ptr<ASTNode> pf_plus(vector<shared_ptr<ASTNode>> &args) {
  double result = 0;
  for (auto &arg : args) {
    if (arg->head->type == TokenType::INT || arg->head->type == TokenType::REAL)
      result += stod(arg->head->value);
    else
      throw RuntimeError(arg->head->span,
                         "plus: invalid argument type " + arg->head->value);
  }

  double intpart;
  shared_ptr<Token> result_token;

  if (modf(result, &intpart) == 0.0)
    result_token = make_shared<Token>(TokenType::INT, to_string((int)result),
                                      args[0]->head->span);
  else
    result_token = make_shared<Token>(TokenType::REAL, to_string(result),
                                      args[0]->head->span);

  return make_shared<ASTNode>(ASTNodeType::LEAF, result_token);
}

shared_ptr<ASTNode> pf_minus(vector<shared_ptr<ASTNode>> &args) {
  double result = 0;
  if (args[0]->head->type == TokenType::INT ||
      args[0]->head->type == TokenType::REAL)
    result = stod(args[0]->head->value);
  else
    throw RuntimeError(args[0]->head->span,
                       "minus: invalid argument type " + args[0]->head->value);

  for (int i = 1; i < args.size(); i++) {
    auto &arg = args[i];
    if (arg->head->type == TokenType::INT || arg->head->type == TokenType::REAL)
      result -= stod(arg->head->value);
    else
      throw RuntimeError(arg->head->span,
                         "minus: invalid argument type " + arg->head->value);
  }

  double intpart;
  shared_ptr<Token> result_token;

  if (modf(result, &intpart) == 0.0)
    result_token = make_shared<Token>(TokenType::INT, to_string((int)result),
                                      args[0]->head->span);
  else
    result_token = make_shared<Token>(TokenType::REAL, to_string(result),
                                      args[0]->head->span);

  return make_shared<ASTNode>(ASTNodeType::LEAF, result_token);
}

shared_ptr<ASTNode> pf_times(vector<shared_ptr<ASTNode>> &args) {
  double result = 1;
  for (auto &arg : args) {
    if (arg->head->type == TokenType::INT || arg->head->type == TokenType::REAL)
      result *= stod(arg->head->value);
    else
      throw RuntimeError(arg->head->span,
                         "times: invalid argument type " + arg->head->value);
  }

  double intpart;
  shared_ptr<Token> result_token;

  if (modf(result, &intpart) == 0.0)
    result_token = make_shared<Token>(TokenType::INT, to_string((int)result),
                                      args[0]->head->span);
  else
    result_token = make_shared<Token>(TokenType::REAL, to_string(result),
                                      args[0]->head->span);

  return make_shared<ASTNode>(ASTNodeType::LEAF, result_token);
}

shared_ptr<ASTNode> pf_divide(vector<shared_ptr<ASTNode>> &args) {
  double result = 0;

  if (args[0]->node_type != ASTNodeType::LEAF)
    throw RuntimeError(args[0]->head->span,
                       "divide: invalid argument type " + args[0]->head->value);

  if (args[0]->head->type == TokenType::INT ||
      args[0]->head->type == TokenType::REAL)
    result = stod(args[0]->head->value);
  else
    throw RuntimeError(args[0]->head->span,
                       "divide: invalid argument type " + args[0]->head->value);

  for (int i = 1; i < args.size(); i++) {
    auto &arg = args[i];
    if (arg->node_type == ASTNodeType::LEAF &&
        (arg->head->type == TokenType::INT ||
         arg->head->type == TokenType::REAL))
      result /= stod(arg->head->value);
    else
      throw RuntimeError(arg->head->span,
                         "divide: invalid argument type " + arg->head->value);
  }

  double intpart;
  shared_ptr<Token> result_token;

  if (modf(result, &intpart) == 0.0)
    result_token = make_shared<Token>(TokenType::INT, to_string((int)result),
                                      args[0]->head->span);
  else
    result_token = make_shared<Token>(TokenType::REAL, to_string(result),
                                      args[0]->head->span);

  return make_shared<ASTNode>(ASTNodeType::LEAF, result_token);
}

shared_ptr<ASTNode> pf_println(vector<shared_ptr<ASTNode>> &args) {
  print_func(args);

  return nullptr;
}

void print_func(vector<shared_ptr<ASTNode>> &args, bool is_recursive) {
  for (auto &arg : args) {
    if (arg->node_type == ASTNodeType::LEAF)
      cout << arg->head->value << ' ';
    else if (arg->node_type == ASTNodeType::LIST ||
             arg->node_type == ASTNodeType::QUOTE_LIST) {
      cout << "'( ";
      print_func(arg->children, true);
      cout << ") ";
    } else
      throw RuntimeError(arg->head->span,
                         "println: invalid argument type " + arg->head->value);
  }
  if (!is_recursive)
    cout << '\n';
}

shared_ptr<ASTNode> pf_equal(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type != args[1]->node_type)
    throw RuntimeError(args[0]->head->span,
                       "equal: invalid argument type " + args[0]->head->value);

  const ASTNodeType &type1 = args[0]->node_type;
  const ASTNodeType &type2 = args[1]->node_type;

  const Token &token1 = *args[0]->head;
  const Token &token2 = *args[1]->head;

  switch (type1) {
  case ASTNodeType::LEAF:
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL,
                           to_string(token1.value == token2.value),
                           token1.span));

  case ASTNodeType::LIST:
    if (args[0]->children.size() != args[1]->children.size())
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", token1.span));

    for (int i = 0; i < args[0]->children.size(); i++) {
      vector<shared_ptr<ASTNode>> arg = {args[0]->children[i],
                                         args[1]->children[i]};
      auto res = pf_equal(args);
      if (res->head->value == "false")
        return make_shared<ASTNode>(
            ASTNodeType::LEAF,
            make_shared<Token>(TokenType::BOOL, "false", token1.span));
    }

    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "true", token1.span));

  default:
    throw RuntimeError(args[0]->head->span,
                       "equal: invalid argument type " + args[0]->head->value);
  }
}

shared_ptr<ASTNode> pf_nonequal(vector<shared_ptr<ASTNode>> &args) {
  return make_shared<ASTNode>(
      ASTNodeType::LEAF,
      make_shared<Token>(TokenType::BOOL,
                         to_string(pf_equal(args)->head->value == "false"),
                         args[0]->head->span));
}

shared_ptr<ASTNode> pf_less(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type != args[1]->node_type)
    throw RuntimeError(args[0]->head->span,
                       "less: invalid argument type " + args[0]->head->value);

  const ASTNodeType &type1 = args[0]->node_type;
  const ASTNodeType &type2 = args[1]->node_type;

  const Token &token1 = *args[0]->head;
  const Token &token2 = *args[1]->head;

  switch (type1) {
  case ASTNodeType::LEAF: {
    double val1 = stod(token1.value);
    double val2 = stod(token2.value);

    return make_shared<ASTNode>(ASTNodeType::LEAF,
                                make_shared<Token>(TokenType::BOOL,
                                                   to_string(val1 < val2),
                                                   token1.span));
  }

  default:
    throw RuntimeError(args[0]->head->span,
                       "less: invalid argument type " + args[0]->head->value);
  }
}

shared_ptr<ASTNode> pf_lesseq(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type != args[1]->node_type)
    throw RuntimeError(args[0]->head->span,
                       "lesseq: invalid argument type " + args[0]->head->value);

  const ASTNodeType &type1 = args[0]->node_type;
  const ASTNodeType &type2 = args[1]->node_type;

  const Token &token1 = *args[0]->head;
  const Token &token2 = *args[1]->head;

  switch (type1) {
  case ASTNodeType::LEAF: {
    double val1 = stod(token1.value);
    double val2 = stod(token2.value);

    return make_shared<ASTNode>(ASTNodeType::LEAF,
                                make_shared<Token>(TokenType::BOOL,
                                                   to_string(val1 <= val2),
                                                   token1.span));
  }

  default:
    throw RuntimeError(args[0]->head->span,
                       "lesseq: invalid argument type " + args[0]->head->value);
  }
}

shared_ptr<ASTNode> pf_greater(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type != args[1]->node_type)
    throw RuntimeError(args[0]->head->span, "greater: invalid argument type " +
                                                args[0]->head->value);

  const ASTNodeType &type1 = args[0]->node_type;
  const ASTNodeType &type2 = args[1]->node_type;

  const Token &token1 = *args[0]->head;
  const Token &token2 = *args[1]->head;

  switch (type1) {
  case ASTNodeType::LEAF: {
    double val1 = stod(token1.value);
    double val2 = stod(token2.value);

    return make_shared<ASTNode>(ASTNodeType::LEAF,
                                make_shared<Token>(TokenType::BOOL,
                                                   to_string(val1 > val2),
                                                   token1.span));
  }

  default:
    throw RuntimeError(args[0]->head->span, "greater: invalid argument type " +
                                                args[0]->head->value);
  }
}

shared_ptr<ASTNode> pf_greatereq(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type != args[1]->node_type)
    throw RuntimeError(args[0]->head->span,
                       "greatereq: invalid argument type " +
                           args[0]->head->value);

  const ASTNodeType &type1 = args[0]->node_type;
  const ASTNodeType &type2 = args[1]->node_type;

  const Token &token1 = *args[0]->head;
  const Token &token2 = *args[1]->head;

  switch (type1) {
  case ASTNodeType::LEAF: {
    double val1 = stod(token1.value);
    double val2 = stod(token2.value);

    return make_shared<ASTNode>(ASTNodeType::LEAF,
                                make_shared<Token>(TokenType::BOOL,
                                                   to_string(val1 >= val2),
                                                   token1.span));
  }

  default:
    throw RuntimeError(args[0]->head->span,
                       "greatereq: invalid argument type " +
                           args[0]->head->value);
  }
}

shared_ptr<ASTNode> pf_and(vector<shared_ptr<ASTNode>> &args) {
  for (auto &arg : args) {
    if (arg->node_type == ASTNodeType::LEAF) {
      if (arg->head->value == "false" || arg->head->value == "0")
        return make_shared<ASTNode>(
            ASTNodeType::LEAF,
            make_shared<Token>(TokenType::BOOL, "false", arg->head->span));
    } else
      throw RuntimeError(arg->head->span,
                         "and: invalid argument type " + arg->head->value);
  }

  return make_shared<ASTNode>(
      ASTNodeType::LEAF,
      make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
}

shared_ptr<ASTNode> pf_or(vector<shared_ptr<ASTNode>> &args) {
  for (auto &arg : args) {
    if (arg->node_type == ASTNodeType::LEAF) {
      if (arg->head->value == "true" || arg->head->value == "1")
        return make_shared<ASTNode>(
            ASTNodeType::LEAF,
            make_shared<Token>(TokenType::BOOL, "true", arg->head->span));
    } else
      throw RuntimeError(arg->head->span,
                         "or: invalid argument type " + arg->head->value);
  }

  return make_shared<ASTNode>(
      ASTNodeType::LEAF,
      make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
}

shared_ptr<ASTNode> pf_not(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LEAF) {
    if (args[0]->head->value == "true" || args[0]->head->value == "1")
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
    else if (args[0]->head->value == "false" || args[0]->head->value == "0")
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
  } else
    throw RuntimeError(args[0]->head->span,
                       "not: invalid argument type " + args[0]->head->value);

  return nullptr;
}

shared_ptr<ASTNode> pf_xor(vector<shared_ptr<ASTNode>> &args) {
  int count = 0;
  for (auto &arg : args) {
    if (arg->node_type == ASTNodeType::LEAF) {
      if (arg->head->value == "true" || arg->head->value == "1")
        count++;
    } else
      throw RuntimeError(arg->head->span,
                         "xor: invalid argument type " + arg->head->value);
  }

  if (count % 2 == 0)
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
  else
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
}

shared_ptr<ASTNode> pf_eval(vector<shared_ptr<ASTNode>> &args) {
  switch (args[0]->node_type) {
  case ASTNodeType::QUOTE_LIST:
    if (args[0]->children[0]->node_type == ASTNodeType::LEAF) {
      return make_shared<FuncCallNode>(
          args[0]->children[0]->head,
          vector<shared_ptr<ASTNode>>(args[0]->children.begin() + 1,
                                      args[0]->children.end()));
    }

    return make_shared<ListNode>(args[0]->children);
  default:
    return args[0];
  }
}

shared_ptr<ASTNode> pf_isint(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LEAF) {
    if (args[0]->head->type == TokenType::INT)
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
    else
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
  } else
    throw RuntimeError(args[0]->head->span,
                       "isint: invalid argument type " + args[0]->head->value);
}

shared_ptr<ASTNode> pf_isreal(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LEAF) {
    if (args[0]->head->type == TokenType::REAL)
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
    else
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
  } else
    throw RuntimeError(args[0]->head->span,
                       "isreal: invalid argument type " + args[0]->head->value);
}

shared_ptr<ASTNode> pf_isbool(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LEAF) {
    if (args[0]->head->type == TokenType::BOOL)
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
    else
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
  } else
    throw RuntimeError(args[0]->head->span,
                       "isbool: invalid argument type " + args[0]->head->value);
}

shared_ptr<ASTNode> pf_isnull(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LEAF) {
    if (args[0]->head->type == TokenType::NUL)
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
    else
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
  } else
    throw RuntimeError(args[0]->head->span,
                       "isnull: invalid argument type " + args[0]->head->value);

  return nullptr;
}

shared_ptr<ASTNode> pf_isatom(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LEAF ||
      args[0]->node_type == ASTNodeType::QUOTE_LIST)
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
  else
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
}

shared_ptr<ASTNode> pf_islist(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LIST)
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
  else
    return make_shared<ASTNode>(
        ASTNodeType::LEAF,
        make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
}

shared_ptr<ASTNode> pf_head(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LIST ||
      args[0]->node_type == ASTNodeType::QUOTE_LIST) {
    if (args[0]->children.size() == 0)
      throw RuntimeError(args[0]->head->span, "head: empty list");

    return args[0]->children[0];
  } else
    throw RuntimeError(args[0]->head->span,
                       "head: invalid argument type " + args[0]->head->value);
}

shared_ptr<ASTNode> pf_tail(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LIST ||
      args[0]->node_type == ASTNodeType::QUOTE_LIST) {
    if (args[0]->children.size() == 0)
      throw RuntimeError(args[0]->head->span, "tail: empty list");

    vector<shared_ptr<ASTNode>> res = args[0]->children;
    res.erase(res.begin());

    return make_shared<ListNode>(res);
  } else
    throw RuntimeError(args[0]->head->span,
                       "tail: invalid argument type " + args[0]->head->value);
}

shared_ptr<ASTNode> pf_cons(vector<shared_ptr<ASTNode>> &args) {
  if (args[1]->node_type == ASTNodeType::LIST ||
      args[1]->node_type == ASTNodeType::QUOTE_LIST) {
    vector<shared_ptr<ASTNode>> res = {args[0]};
    res.insert(res.end(), args[1]->children.begin(), args[1]->children.end());

    return make_shared<ListNode>(res);
  } else
    throw RuntimeError(args[1]->head->span,
                       "cons: invalid argument type " + args[0]->head->value);
}

shared_ptr<ASTNode> pf_isempty(vector<shared_ptr<ASTNode>> &args) {
  if (args[0]->node_type == ASTNodeType::LIST ||
      args[0]->node_type == ASTNodeType::QUOTE_LIST) {
    if (args[0]->children.size() == 0)
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "true", args[0]->head->span));
    else
      return make_shared<ASTNode>(
          ASTNodeType::LEAF,
          make_shared<Token>(TokenType::BOOL, "false", args[0]->head->span));
  } else
    throw RuntimeError(args[0]->head->span, "isempty: invalid argument type " +
                                                args[0]->head->value);
}
