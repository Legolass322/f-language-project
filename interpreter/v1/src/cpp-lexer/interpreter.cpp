#include "ast.h"
#include "interpeter.h"
#include <algorithm>
#include <iostream>
#include <memory>

using namespace interp;

Scope::Scope() {}

Scope::Scope(ASTNodeType scope_type)
    : scope_type(scope_type), return_value(nullptr), break_flag(false) {}

shared_ptr<ASTNode> &Scope::operator[](string const &key) {
  return variables[key];
}

Interpreter::Interpreter() {}

Interpreter::~Interpreter() {}

void Interpreter::eval_result(shared_ptr<ASTNode> const &node,
                              bool is_recursive) {
  if (node == nullptr)
    return;

  if (node->node_type == ASTNodeType::LEAF) {
    cout << node->head->value;
  } else if (node->node_type == ASTNodeType::LIST) {
    cout << "(";
    for (int i = 0; i < node->children.size(); i++) {
      eval_result(node->children[i], true);
      if (i != node->children.size() - 1)
        cout << " ";
    }
    cout << ")";
  } else if (node->node_type == ASTNodeType::QUOTE_LIST) {
    cout << "'";
    for (int i = 0; i < node->children.size(); i++) {
      eval_result(node->children[i], true);
      if (i != node->children.size() - 1)
        cout << " ";
    }
  } else {
    cout << "(" << node->head->value;
    cout << " ";
    for (int i = 0; i < node->children.size(); i++) {
      eval_result(node->children[i], true);
      if (i != node->children.size() - 1)
        cout << " ";
    }
    cout << ")";
  }
}

shared_ptr<ASTNode> Interpreter::interpret(shared_ptr<ASTNode> const &node) {
  switch (node->node_type) {
  case ASTNodeType::PROGRAM:
    interpret_program(node);
    break;
  case ASTNodeType::FUNCDEF:
    interpret_funcdef(static_pointer_cast<FuncDefNode>(node));
    break;
  case ASTNodeType::FUNCCALL:
    return interpret_funccall(static_pointer_cast<FuncCallNode>(node));
  case ASTNodeType::LAMBDA:
    interpret_lambda(static_pointer_cast<LambdaNode>(node));
    break;
  case ASTNodeType::LIST:
    return interpret_list(static_pointer_cast<ListNode>(node));
  case ASTNodeType::QUOTE_LIST:
    return interpret_list(static_pointer_cast<ListNode>(node));
  case ASTNodeType::RETURN:
    return interpret_return(static_pointer_cast<ReturnNode>(node));
  case ASTNodeType::BREAK:
    interpret_break(node);
    break;
  case ASTNodeType::COND:
    return interpret_cond(static_pointer_cast<CondNode>(node));
  case ASTNodeType::WHILE:
    interpret_while(static_pointer_cast<WhileNode>(node));
    break;
  case ASTNodeType::PROG:
    return interpret_prog(static_pointer_cast<ProgNode>(node));
  case ASTNodeType::SETQ:
    interpret_setq(static_pointer_cast<SetqNode>(node));
    break;
  case ASTNodeType::LEAF:
    cout << "found leaf: " << node->head->value << '\n';
    return interpret_leaf(node);
  }

  return nullptr;
}

void Interpreter::interpret_program(shared_ptr<ASTNode> const &node) {
  if (stack.empty()) {
    stack.push_back(Scope(ASTNodeType::PROGRAM));
  }

  for (int i = 0; i < node->children.size() - 1; i++) {

    interpret(node->children[i]);
  }

  eval_result(interpret(node->children.back()));
}

void Interpreter::interpret_funcdef(shared_ptr<FuncDefNode> const &node) {
  auto const &name = node->getName()->value;
  stack.back()[name] = node;
}

shared_ptr<ASTNode>
Interpreter::interpret_funccall(shared_ptr<FuncCallNode> const &node) {
  auto const &name = node->getName()->value;
  auto const &args = node->getArgs();

  cout << "calling function " << name << '\n';

  if (find(PF_FUNCS.begin(), PF_FUNCS.end(), name) != PF_FUNCS.end()) {
    cout << "calling pf func" << '\n';
    vector<shared_ptr<ASTNode>> v_args;
    for (auto const &arg : args) {
      v_args.push_back(interpret(arg));
    }
    return PF_FUNC_MAP.at(name)(v_args);
  }

  auto const &params = node->node_type == ASTNodeType::FUNCCALL
                           ? stack.back()[name]->children[1]
                           : stack.back()[name]->children[0];
  auto const &body = node->node_type == ASTNodeType::FUNCCALL
                         ? stack.back()[name]->children[2]
                         : stack.back()[name]->children[1];

  stack.push_back(Scope(ASTNodeType::FUNCCALL));

  for (int i = 0; i < params->children.size(); i++) {
    stack.back()[params->children[i]->head->value] = args[i];
  }

  shared_ptr<ASTNode> res = interpret(body);

  stack.pop_back();

  return res;
}

void Interpreter::interpret_setq(shared_ptr<SetqNode> const &node) {
  auto const &name = node->getName()->value;
  auto const &value = node->getValue();

  cout << "setting variable " << name << "with value " << value->head->value
       << '\n';

  cout << "var size: " << stack.back().variables.size() << '\n';

  for (int i = stack.size() - 1; i >= 0; i--) {
    if (stack[i].variables.find(name) != stack[i].variables.end()) {
      stack[i][name] = interpret(value);
      return;
    }
  }

  stack.back()[name] = interpret(value);

  cout << "var size: " << stack.back().variables.size() << '\n';
}

void Interpreter::interpret_break(shared_ptr<ASTNode> const &node) {
  stack.back().break_flag = true;
}

void Interpreter::interpret_while(shared_ptr<WhileNode> const &node) {
  stack.push_back(Scope(ASTNodeType::WHILE));
  while (true) {
    auto cond_res = interpret(node->getCond());

    if (stack.back().break_flag) {
      break;
    }

    if (cond_res->node_type == ASTNodeType::LEAF &&
        (cond_res->head->value != "false" && cond_res->head->value != "0")) {
      interpret(node->getBody());
    } else {
      break;
    }
  }
  stack.pop_back();
}

void Interpreter::interpret_lambda(shared_ptr<LambdaNode> const &node) {}

shared_ptr<ASTNode>
Interpreter::interpret_list(shared_ptr<ListNode> const &node) {
  if (node->node_type == ASTNodeType::QUOTE_LIST)
    return node;

  vector<shared_ptr<ASTNode>> res;
  for (auto const &child : node->children) {
    res.push_back(interpret(child));
  }
  return make_shared<ListNode>(res);
}

shared_ptr<ASTNode>
Interpreter::interpret_return(shared_ptr<ReturnNode> const &node) {
  stack.back().return_value = interpret(node->getValue());
  return stack.back().return_value;
}

shared_ptr<ASTNode>
Interpreter::interpret_cond(shared_ptr<CondNode> const &node) {
  auto cond_res = interpret(node->getCond());

  if (cond_res->node_type == ASTNodeType::LEAF &&
      (cond_res->head->value == "true" || cond_res->head->value == "1")) {
    return interpret(node->getBranchTrue());
  } else {
    return interpret(node->getBranchFalse());
  }
}

shared_ptr<ASTNode>
Interpreter::interpret_prog(shared_ptr<ProgNode> const &node) {
  stack.push_back(Scope(ASTNodeType::PROG));

  for (int i = 1; i < node->children.size() - 1; i++) {
    if (stack.back().return_value) {
      auto res = stack.back().return_value;
      stack.pop_back();
      return res;
    }

    if (stack.back().break_flag) {
      stack.pop_back();
      stack.back().break_flag = true;
      return nullptr;
    }

    interpret(node->children[i]);
  }

  // return last statement result in case of no return statement
  if (!stack.back().return_value) {
    cout << "no return statement" << '\n';
    auto res = interpret(node->children.back());

    if (stack.back().break_flag) {
      stack.pop_back();
      stack.back().break_flag = true;
      cout << "encountered break in last stmt" << '\n';
      return nullptr;
    }

    stack.pop_back();

    return res;
  }

  auto res = stack.back().return_value;
  stack.pop_back();
  return res;
}

shared_ptr<ASTNode>
Interpreter::interpret_leaf(shared_ptr<ASTNode> const &node) {
  cout << "interpreting leaf" << '\n';
  if (node->head->type == TokenType::IDENTIFIER) {
    for (int i = stack.size() - 1; i >= 0; i--) {
      if (stack[i].variables.find(node->head->value) !=
          stack[i].variables.end()) {
        cout << "returning variable" << '\n';
        return stack[i][node->head->value];
      }
    }
  }

  cout << "returning node" << '\n';
  return node;
}
