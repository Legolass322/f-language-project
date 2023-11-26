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
      throw RuntimeError(arg->head->span, "invalid argument type");
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
  cout << "minus" << '\n';
  double result = 0;
  if (args[0]->head->type == TokenType::INT ||
      args[0]->head->type == TokenType::REAL)
    result = stod(args[0]->head->value);
  else
    throw RuntimeError(args[0]->head->span, "invalid argument type");

  for (int i = 1; i < args.size(); i++) {
    auto &arg = args[i];
    if (arg->head->type == TokenType::INT || arg->head->type == TokenType::REAL)
      result -= stod(arg->head->value);
    else
      throw RuntimeError(arg->head->span, "invalid argument type");
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
      throw RuntimeError(arg->head->span, "invalid argument type");
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
    throw RuntimeError(args[0]->head->span, "invalid argument type");

  if (args[0]->head->type == TokenType::INT ||
      args[0]->head->type == TokenType::REAL)
    result = stod(args[0]->head->value);
  else
    throw RuntimeError(args[0]->head->span, "invalid argument type");

  for (int i = 1; i < args.size(); i++) {
    auto &arg = args[i];
    if (arg->node_type == ASTNodeType::LEAF &&
        (arg->head->type == TokenType::INT ||
         arg->head->type == TokenType::REAL))
      result /= stod(arg->head->value);
    else
      throw RuntimeError(arg->head->span, "invalid argument type");
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
  for (auto &arg : args) {
    if (arg->node_type == ASTNodeType::LEAF)
      cout << arg->head->value << ' ';
    else
      throw RuntimeError(arg->head->span, "invalid argument type");
  }
  cout << '\n';

  return nullptr;
}
