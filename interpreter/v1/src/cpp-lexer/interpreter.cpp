#include "interpeter.h"

using namespace interp;

interp::Scope::Scope() {}

interp::Scope::Scope(ASTNodeType scope_type, bool inlined)
    : scope_type(scope_type), return_value(nullptr), break_flag(false),
      inlined(inlined) {}

shared_ptr<ASTNode> &interp::Scope::operator[](string const &key) {
  return variables[key];
}

Interpreter::Interpreter() {}

Interpreter::~Interpreter() {}

void Interpreter::eval_result(shared_ptr<ASTNode> const &node,
                              bool is_recursive) {
  if (node == nullptr)
    return;

  switch (node->node_type) {
  case ASTNodeType::LIST: {
    auto list_node = static_pointer_cast<ListNode>(node);
    wcout << *list_node.get();
    break;
  }

  case ASTNodeType::QUOTE_LIST: {
    wcout << "'";
    auto quote_list_node = static_pointer_cast<ListNode>(node);
    wcout << *quote_list_node.get();
    break;
  }
  case ASTNodeType::FUNCDEF: {
    auto funcdef_node = static_pointer_cast<FuncDefNode>(node);
    wcout << *funcdef_node.get();
    break;
  }
  case ASTNodeType::FUNCCALL: {
    auto funccall_node = static_pointer_cast<FuncCallNode>(node);
    wcout << *funccall_node.get();
    break;
  }
  case ASTNodeType::LAMBDA: {
    auto lambda_node = static_pointer_cast<LambdaNode>(node);
    wcout << *lambda_node.get();
    break;
  }

  case ASTNodeType::RETURN: {
    auto return_node = static_pointer_cast<ReturnNode>(node);
    wcout << *return_node.get();
    break;
  }

  case ASTNodeType::COND: {
    auto cond_node = static_pointer_cast<CondNode>(node);
    wcout << *cond_node.get();
    break;
  }

  case ASTNodeType::WHILE: {
    auto while_node = static_pointer_cast<WhileNode>(node);
    wcout << *while_node.get();
    break;
  }

  case ASTNodeType::PROG: {
    auto prog_node = static_pointer_cast<ProgNode>(node);
    wcout << *prog_node.get();
    break;
  }

  case ASTNodeType::SETQ: {
    auto setq_node = static_pointer_cast<SetqNode>(node);
    wcout << *setq_node.get();
    break;
  }

  default:
    wcout << *node.get();
    break;
  }

  if (!is_recursive) {
    wcout << endl;
  }
}

shared_ptr<ASTNode> Interpreter::interpret(shared_ptr<ASTNode> const &node) {
  if (node == nullptr)
    return nullptr;

  switch (node->node_type) {
  case ASTNodeType::PROGRAM:
    interpret_program(node);
    break;
  case ASTNodeType::FUNCDEF:
    return interpret_funcdef(static_pointer_cast<FuncDefNode>(node));
  case ASTNodeType::FUNCCALL:
    return interpret_funccall(static_pointer_cast<FuncCallNode>(node));
  case ASTNodeType::LAMBDA:
    return interpret_lambda(static_pointer_cast<LambdaNode>(node));
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

shared_ptr<ASTNode>
Interpreter::interpret_funcdef(shared_ptr<FuncDefNode> const &node) {
  auto const &name = node->getName()->value;
  stack.back()[name] = interpret_func_closure(node);

  return stack.back()[name];
}

shared_ptr<ASTNode>
Interpreter::interpret_funccall(shared_ptr<FuncCallNode> const &node) {
  if (node->children[0]->node_type != LEAF) {

    auto head_node = interpret(node->children[0]);

    stack.push_back(Scope(ASTNodeType::FUNCCALL));

    switch (head_node->node_type) {
    case ASTNodeType::FUNCDEF: {
      auto const &name = head_node->children[0]->head;
      interpret_funcdef(static_pointer_cast<FuncDefNode>(head_node));
      vector<shared_ptr<ASTNode>> v_args = {
          make_shared<ASTNode>(ASTNodeType::LEAF, name)};
      for (auto const &arg : node->getArgs()) {
        v_args.push_back(arg);
      }
      auto res = interpret_funccall(
          make_shared<FuncCallNode>(v_args[0]->head, v_args));
      stack.pop_back();
      return res;
    }

    case ASTNodeType::LAMBDA: {
      auto const &params = head_node->children[0];
      auto const &body = head_node->children[1];
      auto const &args = node->getArgs();

      for (int i = 0; i < params->children.size(); i++) {
        stack.back()[params->children[i]->head->value] = interpret(args[i]);
      }

      auto res = interpret(body);

      stack.pop_back();

      return res;
    }

    case ASTNodeType::LEAF: {
      auto const &name = head_node->head;
      auto const &args = node->getArgs();

      vector<shared_ptr<ASTNode>> v_args = {
          make_shared<ASTNode>(ASTNodeType::LEAF, name)};

      for (auto const &arg : args) {
        v_args.push_back(arg);
      }

      auto res = interpret_funccall(
          make_shared<FuncCallNode>(v_args[0]->head, v_args));

      stack.pop_back();

      return res;
    }

    default:
      stack.pop_back();
      throw runtime_error("not a function");
    }

  } else {

    auto const &name = node->getName()->value;
    auto const &args = node->getArgs();

    if (find(PF_FUNCS.begin(), PF_FUNCS.end(), name) != PF_FUNCS.end()) {
      vector<shared_ptr<ASTNode>> v_args;
      for (auto const &arg : args) {
        v_args.push_back(interpret(arg));
      }
      return interpret(PF_FUNC_MAP.at(name)(v_args));
    }

    shared_ptr<ASTNode> funcdef;

    for (int i = stack.size() - 1; i >= 0; i--) {
      if (stack[i].variables.find(name) != stack[i].variables.end()) {
        funcdef = stack[i][name];
        break;
      }
    }

    if (funcdef == nullptr) {
      throw runtime_error(name + " is not a function");
    }

    if (funcdef->node_type != ASTNodeType::FUNCDEF &&
        funcdef->node_type != ASTNodeType::LAMBDA) {

      if (funcdef->node_type == ASTNodeType::LEAF &&
          funcdef->head->type == TokenType::IDENTIFIER) {
        vector<shared_ptr<ASTNode>> v_args = {
            make_shared<ASTNode>(ASTNodeType::LEAF, funcdef->head)};

        for (auto const &arg : args) {
          v_args.push_back(arg);
        }

        return interpret_funccall(
            make_shared<FuncCallNode>(v_args[0]->head, v_args));
      }

      throw runtime_error(name + " is not a function");
    }

    auto const &params = funcdef->node_type == ASTNodeType::FUNCDEF
                             ? funcdef->children[1]
                             : funcdef->children[0];
    auto const &body = funcdef->node_type == ASTNodeType::FUNCDEF
                           ? funcdef->children[2]
                           : funcdef->children[1];

    stack.push_back(Scope(ASTNodeType::FUNCCALL));

    for (int i = 0; i < params->children.size(); i++) {
      stack.back()[params->children[i]->head->value] = interpret(args[i]);
    }

    if (body->node_type == ASTNodeType::PROG) {
      for (auto &child : body->children) {
        if (child->node_type == ASTNodeType::SETQ) {
          stack.back()[child->children[0]->head->value] = nullptr;
        }
      }
    }

    shared_ptr<ASTNode> res = interpret(body);

    stack.pop_back();

    return res;
  }
}

void Interpreter::interpret_setq(shared_ptr<SetqNode> const &node) {
  auto const &name = node->getName()->value;
  auto const &value = node->getValue();

  for (int i = stack.size() - 1; i >= 0; i--) {
    if (stack[i].variables.find(name) != stack[i].variables.end()) {
      stack[i][name] = interpret(value);
      return;
    }

    if (stack[i].inlined) {
      break;
    }
  }

  stack.back()[name] = interpret(value);
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

shared_ptr<ASTNode>
Interpreter::interpret_lambda(shared_ptr<LambdaNode> const &node) {
  return interpret_lambda_closure(node);
}

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
Interpreter::interpret_lambda_closure(shared_ptr<LambdaNode> const &node) {
  shared_ptr<LambdaNode> lambda = static_pointer_cast<LambdaNode>(node->copy());
  vector<shared_ptr<ASTNode>> setqs;
  vector<string> defined;

  for (auto const &param : lambda->getParams()->children) {
    defined.push_back(param->head->value);
  }

  if (lambda->getBody()->node_type == ASTNodeType::PROG) {
    for (auto const &child : lambda->getBody()->children) {
      if (child->node_type == ASTNodeType::SETQ) {
        auto setq_node = static_pointer_cast<SetqNode>(child);
        defined.push_back(setq_node->getName()->value);
      }
    }
  }

  iterate_closure(lambda->getBody(), setqs, defined);

  if (lambda->getBody()->node_type == ASTNodeType::PROG) {
    auto body = static_pointer_cast<ProgNode>(lambda->getBody());
    body->children.insert(body->children.begin() + 1, setqs.begin(),
                          setqs.end());

    return lambda;
  }

  vector<shared_ptr<ASTNode>> children = {make_shared<ListNode>()};

  // add setqs to locals
  for (auto const &setq : setqs) {
    children.push_back(setq);
    children[0]->children.push_back(
        make_shared<ASTNode>(ASTNodeType::LEAF, setq->children[0]->head));
  }

  children.push_back(lambda->getBody());

  lambda->setBody(make_shared<ProgNode>(
      make_shared<Token>(TokenType::KEYWORD, "prog", Span({0, 0})), children));

  generate_graph_svg(lambda, "closure.svg");

  return lambda;
}

shared_ptr<ASTNode>
Interpreter::interpret_func_closure(shared_ptr<FuncDefNode> const &node) {
  shared_ptr<FuncDefNode> funcdef =
      static_pointer_cast<FuncDefNode>(node->copy());
  vector<shared_ptr<ASTNode>> setqs;
  vector<string> defined;

  for (auto const &param : funcdef->getParams()->children) {
    defined.push_back(param->head->value);
  }

  if (funcdef->getBody()->node_type == ASTNodeType::PROG) {
    for (auto const &child : funcdef->getBody()->children) {
      if (child->node_type == ASTNodeType::SETQ) {
        auto setq_node = static_pointer_cast<SetqNode>(child);
        defined.push_back(setq_node->getName()->value);
      }
    }
  }

  iterate_closure(funcdef->getBody(), setqs, defined);

  if (funcdef->getBody()->node_type == ASTNodeType::PROG) {
    auto body = static_pointer_cast<ProgNode>(funcdef->getBody());
    body->children.insert(body->children.begin() + 1, setqs.begin(),
                          setqs.end());

    return funcdef;
  }

  vector<shared_ptr<ASTNode>> children = {make_shared<ListNode>()};

  // add setqs to locals
  for (auto const &setq : setqs) {
    children.push_back(setq);
    children[0]->children.push_back(
        make_shared<ASTNode>(ASTNodeType::LEAF, setq->children[0]->head));
  }

  children.push_back(funcdef->getBody());

  funcdef->setBody(make_shared<ProgNode>(
      make_shared<Token>(TokenType::KEYWORD, "prog", Span({0, 0})), children));

  generate_graph_svg(funcdef, "closure.svg");

  return funcdef;
}

void Interpreter::iterate_closure(shared_ptr<ASTNode> const &node,
                                  vector<shared_ptr<ASTNode>> &setqs,
                                  vector<string> const &defined) {
  if (node->node_type == ASTNodeType::LEAF) {
    if (node->head->type == TokenType::IDENTIFIER) {
      if (find(defined.begin(), defined.end(), node->head->value) ==
          defined.end()) {
        auto res = interpret_leaf(node);

        if (res->head->value == node->head->value)
          return;

        vector<shared_ptr<ASTNode>> children = {
            make_shared<ASTNode>(ASTNodeType::LEAF, node->head), res};

        auto setq_node = make_shared<SetqNode>(
            make_shared<Token>(TokenType::KEYWORD, "setq", Span({0, 0})),
            children);

        setqs.push_back(setq_node);
        return;
      }
    }
  }

  for (auto const &child : node->children) {
    iterate_closure(child, setqs, defined);
  }
}

shared_ptr<ASTNode>
Interpreter::interpret_cond(shared_ptr<CondNode> const &node) {
  auto cond_res = interpret(node->getCond());

  if (cond_res->node_type == ASTNodeType::LEAF &&
      (cond_res->head->value == "true" || cond_res->head->value == "1")) {
    return interpret(node->getBranchTrue());
  } else if (node->getBranchFalse() != nullptr) {
    return interpret(node->getBranchFalse());
  }

  return nullptr;
}

shared_ptr<ASTNode>
Interpreter::interpret_prog(shared_ptr<ProgNode> const &node) {
  stack.push_back(Scope(ASTNodeType::PROG, node->is_inlined));

  for (auto &loc : node->getLocals()->children) {
    stack.back()[loc->head->value] = nullptr;
  }

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
    auto res = interpret(node->children.back());

    if (stack.back().break_flag) {
      stack.pop_back();
      stack.back().break_flag = true;
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
  if (node->head->type == TokenType::IDENTIFIER) {
    for (int i = stack.size() - 1; i >= 0; i--) {
      if (stack[i].variables.find(node->head->value) !=
          stack[i].variables.end()) {
        return stack[i][node->head->value];
      }
    }
  }

  return node;
}
