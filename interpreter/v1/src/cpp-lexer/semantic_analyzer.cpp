#include "semantic_analyzer.h"
#include "ast.h"
#include "driver.hh"
#include <algorithm>
#include <iostream>
#include <memory>

// #define DEBUG

#ifdef DEBUG
using std::cout, std::endl;
#endif

SemanticAnalyzer::SemanticAnalyzer() { tmp_counter = 0; }

SemanticAnalyzer::~SemanticAnalyzer() {}

shared_ptr<ASTNode> SemanticAnalyzer::inline_require(shared_ptr<ASTNode> node) {
  if (node->node_type == QUOTE_LIST) {
    string filename;

    for (auto &child : node->children) {
      filename += child->head->value;
    }

    filename += ".flang";

    FILE *file = fopen(filename.c_str(), "r");

    if (file == nullptr)
      throw RuntimeError(node->head->span, "File not found");

    Driver drv;
    drv.parse(filename);

    auto ast = drv.ast;

    if (ast == nullptr)
      throw RuntimeError(node->head->span, "File not found");

    return ast;
  }

  throw RuntimeError(node->head->span, "Invalid require statement");
}

void SemanticAnalyzer::clear_stack(shared_ptr<ASTNode> &root) {
  if (root == nullptr)
    return;

  while (scope_stack.size() > 1) {
    scope_stack.pop_back();
  }

  for (auto &child : root->children) {
    if (child->node_type == FUNCDEF) {
      auto funcdef_node = static_pointer_cast<FuncDefNode>(child);
      auto name = funcdef_node->getName()->value;

      if (scope_stack.back().variables.find(name) !=
          scope_stack.back().variables.end()) {
        scope_stack.back().variables.erase(name);
      }
    } else if (child->node_type == SETQ) {
      auto setq_node = static_pointer_cast<SetqNode>(child);
      auto name = setq_node->getName()->value;

      if (scope_stack.back().variables.find(name) !=
          scope_stack.back().variables.end()) {
        scope_stack.back().variables.erase(name);
      }
    }
  }
}

void SemanticAnalyzer::analyze(shared_ptr<ASTNode> &root) {
  if (root == nullptr)
    return;

  if (scope_stack.size() == 0)
    scope_stack.push_back(Scope(root));

  // entry point for semantic analysis
  for (int i = 0; i < root->children.size(); ++i) {
    auto &node = root->children[i];

    // check for require statements
    if (node->node_type == FUNCCALL) {
      auto func_call = static_pointer_cast<FuncCallNode>(node);
      if (func_call->getName()->value == "require") {
        shared_ptr<ASTNode> ast = inline_require(func_call->getArgs()[0]);

        // remove require statement
        root->children.erase(root->children.begin() + i);

        for (auto &child : ast->children) {
          root->children.insert(root->children.begin() + i, child);
        }

        continue;
      }
    }

    node = analyze_node(node);
  }
}

Var SemanticAnalyzer::find_variable(shared_ptr<Token> identifier) {
  string identifier_str = identifier->value;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto variable = it->variables.find(identifier_str);
    if (variable != it->variables.end()) {
      variable->second.referrers++; // increment referrers, since we are using
                                    // this var
      return variable->second;
    }
  }

  if (find(PF_FUNCTIONS.begin(), PF_FUNCTIONS.end(), identifier_str) !=
      PF_FUNCTIONS.end())
    return Var(make_shared<ASTNode>(LEAF, identifier));
#ifdef DEBUG
  cout << "error in find " << identifier_str << endl;
#endif
  throw VariableNotFoundError(identifier->span, identifier_str);
}

shared_ptr<ASTNode>
SemanticAnalyzer::remove_variable(shared_ptr<ASTNode> node, Scope &scope,
                                  string const &identifier) {
  auto variable = scope.variables.find(identifier);

  if (variable != scope.variables.end() && variable->second.referrers == 0) {
    for (int i = 0; i < node->children.size(); ++i) {
      if (node->children[i]->node_type == SETQ) {
        auto setq_node = static_pointer_cast<SetqNode>(node->children[i]);
        if (setq_node->getName()->value == identifier) {
          node->children.erase(node->children.begin() + i);
          return node;
        }
      }
    }
  }

#ifdef DEBUG
  cout << "error in remove var" << identifier << endl;
#endif
  throw VariableNotFoundError(node->head->span, identifier);
}

shared_ptr<ASTNode> SemanticAnalyzer::remove_inlined_function(
    shared_ptr<ASTNode> node, Scope &scope, string const &identifier) {
  auto variable = scope.variables.find(identifier);

  if (variable != scope.variables.end()) {
    for (int i = 0; i < node->children.size(); ++i) {
      if (node->children[i]->node_type == FUNCDEF) {
        auto funcdef_node = static_pointer_cast<FuncDefNode>(node->children[i]);
        if (funcdef_node->getName()->value == identifier) {
          if (funcdef_node->is_recursive)
            return node;

          node->children.erase(node->children.begin() + i);
          return node;
        }
      }
    }
  }

#ifdef DEBUG
  cout << "error in remove func" << identifier << endl;
#endif

  throw VariableNotFoundError(node->head->span, identifier);
}

shared_ptr<ASTNode> SemanticAnalyzer::get_inlined_function(
    shared_ptr<Token> const &identifier,
    vector<shared_ptr<ASTNode>> const &args) {
  shared_ptr<FuncDefNode> func_copy =
      static_pointer_cast<FuncDefNode>(find_variable(identifier).value->copy());
  shared_ptr<ASTNode> node_body = func_copy->getBody();
  vector<shared_ptr<ASTNode>> params = func_copy->getParams()->children;
  map<string, string> tmps;
  map<string, shared_ptr<ASTNode>> params_to_args;
  vector<shared_ptr<ASTNode>> add_to_body_locals;

  // check if number of arguments match number of parameters
  if (args.size() != params.size())
    throw WrongNumberOfArgumentsError(func_copy->head->span, identifier->value,
                                      params.size(), args.size());

#ifdef DEBUG
  cout << "get_inlined_function" << endl;
#endif

  // check for recursive calls
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    if (it->scope_node->node_type == FUNCDEF) {
      auto funcdef_node = static_pointer_cast<FuncDefNode>(it->scope_node);
      if (funcdef_node->getName()->value == identifier->value) {
        funcdef_node->is_recursive = true;

#ifdef DEBUG
        cout << "recursive call found" << endl;
#endif
        return make_shared<FuncCallNode>(identifier, args);
      }
    }
  }

  // create tmp variables for each parameter
  for (int i = 0; i < params.size(); i++) {
    string tmp = "_tmp" + to_string(++tmp_counter);

    tmps[params[i]->head->value] = tmp;

    params_to_args[params[i]->head->value] = args[i];
  }

  for (auto &child : node_body->children) {
    child = inline_function(child, tmps, params_to_args);
    if (child->node_type == SETQ) {
      auto setq_node = static_pointer_cast<SetqNode>(child);
      add_to_body_locals.push_back(
          make_shared<ASTNode>(LEAF, setq_node->getName()));
    }
  }

  // create setq nodes for each parameter
  vector<shared_ptr<ASTNode>> setqs;
  vector<shared_ptr<ASTNode>> locals;
  for (auto it = tmps.begin(); it != tmps.end(); ++it) {
    locals.push_back(make_shared<ASTNode>(
        LEAF, make_shared<Token>(IDENTIFIER, it->second, identifier->span)));
  }
  setqs.push_back(make_shared<ListNode>(locals));

  for (int i = 0; i < args.size(); i++) {
    if (tmps.find(params[i]->head->value) == tmps.end())
      continue;

    vector<shared_ptr<ASTNode>> setq_children = {
        make_shared<ASTNode>(
            LEAF, make_shared<Token>(IDENTIFIER, tmps[params[i]->head->value],
                                     params[i]->head->span)),
        args[i]};
    setqs.push_back(make_shared<SetqNode>(params[i]->head, setq_children));
  }

  vector<shared_ptr<ASTNode>> body_children;

  for (auto &setq : setqs) {
    body_children.push_back(setq);
  }

  if (add_to_body_locals.size() > 0 && node_body->node_type == PROG) {
    node_body->children[0] = make_shared<ListNode>(add_to_body_locals);
  }

  body_children.push_back(node_body);

  return make_shared<ProgNode>(node_body->head, body_children);
}

shared_ptr<ASTNode> SemanticAnalyzer::inline_function(
    shared_ptr<ASTNode> node, map<string, string> const &tmps,
    map<string, shared_ptr<ASTNode>> const &params_to_args) {

#ifdef DEBUG
  cout << "inline_function" << endl;
#endif

  if (node->node_type == LEAF) {
    if (node->head->type == IDENTIFIER) {
      auto tmp = tmps.find(node->head->value);
      if (tmp != tmps.end()) {

#ifdef DEBUG
        cout << "replacing " << node->head->value << " with " << tmp->second
             << endl;
#endif
        return make_shared<ASTNode>(
            LEAF,
            make_shared<Token>(IDENTIFIER, tmp->second, node->head->span));
      }
    }
  } else if (node->node_type == FUNCCALL) { // if argument is a function
    auto func_call = static_pointer_cast<FuncCallNode>(node);
    auto arg = params_to_args.find(func_call->getName()->value);
    if (arg != params_to_args.end()) {
#ifdef DEBUG
      cout << "replacing " << func_call->getName()->value << " with "
           << arg->second->head->value << endl;
#endif

      if (arg->second->node_type == LEAF &&
          arg->second->head->type == IDENTIFIER) {
        return make_shared<FuncCallNode>(arg->second->head,
                                         func_call->children);
      } else if (arg->second->node_type == LAMBDA) {
        auto lambda_node = static_pointer_cast<LambdaNode>(arg->second);

        return get_inlined_lambda(lambda_node, func_call->getArgs());
      }
    }
  }

  for (auto &child : node->children) {
    child = inline_function(child, tmps, params_to_args);
  }

  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::get_inlined_lambda(shared_ptr<LambdaNode> const &lambda,
                                     vector<shared_ptr<ASTNode>> const &args) {
  shared_ptr<LambdaNode> lambda_copy =
      static_pointer_cast<LambdaNode>(lambda->copy());
  shared_ptr<ASTNode> node_body = lambda_copy->getBody();
  vector<shared_ptr<ASTNode>> params = lambda_copy->getParams()->children;
  map<string, string> tmps;
  map<string, shared_ptr<ASTNode>> params_to_args;

  // check if number of arguments match number of parameters
  if (args.size() != params.size())
    throw WrongNumberOfArgumentsError(lambda->head->span, "lambda",
                                      params.size(), args.size());

  // create tmp variables for each parameter
  for (int i = 0; i < params.size(); i++) {
    string tmp = "_tmp" + to_string(++tmp_counter);

    tmps[params[i]->head->value] = tmp;
    params_to_args[params[i]->head->value] = args[i];
  }

  for (auto &child : node_body->children) {
    child = inline_function(child, tmps, params_to_args);
  }

  // create setq nodes for each parameter
  vector<shared_ptr<ASTNode>> setqs;
  vector<shared_ptr<ASTNode>> locals;
  for (auto it = tmps.begin(); it != tmps.end(); ++it) {
    locals.push_back(make_shared<ASTNode>(
        LEAF, make_shared<Token>(IDENTIFIER, it->second, lambda->head->span)));
  }
  setqs.push_back(make_shared<ListNode>(locals));

  for (int i = 0; i < args.size(); i++) {
    vector<shared_ptr<ASTNode>> setq_children = {
        make_shared<ASTNode>(
            LEAF, make_shared<Token>(IDENTIFIER, tmps[params[i]->head->value],
                                     params[i]->head->span)),
        args[i]};
    setqs.push_back(make_shared<SetqNode>(params[i]->head, setq_children));
  }

  vector<shared_ptr<ASTNode>> body_children;

  for (auto &setq : setqs) {
    body_children.push_back(setq);
  }

  body_children.push_back(node_body);

  return make_shared<ProgNode>(node_body->head, body_children);
}

shared_ptr<ASTNode>
SemanticAnalyzer::find_function(shared_ptr<Token> identifier) {
  string identifier_str = identifier->value;

  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto variable = it->variables.find(identifier_str);
    if (variable != it->variables.end()) {
      variable->second.referrers++; // increment referrers, since we are using
                                    // this var
      return variable->second.value;
    }
  }

  throw FunctionNotFoundError(identifier->span, identifier_str);
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_node(shared_ptr<ASTNode> node) {
#ifdef DEBUG
  cout << "analyze_node " << node->head->value << endl;
#endif
  switch (node->node_type) {
  case FUNCDEF:
    return analyze_funcdef(static_pointer_cast<FuncDefNode>(node));
  case FUNCCALL:
    return analyze_funccall(static_pointer_cast<FuncCallNode>(node));
  case LAMBDA:
    return analyze_lambda(static_pointer_cast<LambdaNode>(node));
  case PROG:
    return analyze_prog(static_pointer_cast<ProgNode>(node));
  case SETQ:
    return analyze_setq(static_pointer_cast<SetqNode>(node));
  case RETURN:
    return analyze_return(static_pointer_cast<ReturnNode>(node));
  case WHILE:
    return analyze_while(static_pointer_cast<WhileNode>(node));
  case COND:
    return analyze_cond(static_pointer_cast<CondNode>(node));
  case BREAK:
    return analyze_break(node);
  case QUOTE_LIST:
    return node;
  default:
    break;
  }

  if (node->head != nullptr && node->head->type == IDENTIFIER) {
#ifdef DEBUG
    cout << "analyze_node " << node->head->value << endl;
#endif
    find_variable(node->head);
    return node;
  }

  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_funcdef(shared_ptr<FuncDefNode> node) {
  string identifier = node->getName()->value;

#ifdef DEBUG
  cout << "analyze_funcdef " << identifier << endl;
#endif

  try {
    find_function(node->getName());
    throw FunctionAlreadyDefinedError(node->getName()->span, identifier);
  } catch (FunctionNotFoundError &e) {
    // Function not found, add it to the list of functions
  }

#ifdef DEBUG
  cout << "adding function " << identifier << endl;
#endif
  scope_stack.back().variables[identifier] = node;

  // create new scope for function
  scope_stack.push_back(Scope(node));

  // add parameters to function scope
  for (auto &child : node->getParams()->children) {
    string identifier = child->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

#ifdef DEBUG
    cout << "adding variable " << identifier << endl;
#endif
    scope.variables[identifier] = child;
  }

  node->setBody(analyze_node(node->getBody()));

  // remove variables that has 0 referrers
  for (auto it = scope_stack.back().variables.rbegin();
       it != scope_stack.back().variables.rend(); ++it) {
    if (it->second.referrers == 0) {
#ifdef DEBUG
      cout << "removing variable " << it->first << endl;
#endif

      bool is_defined = false;

      try {
        find_variable(
            make_shared<Token>(IDENTIFIER, it->first, node->head->span));
        is_defined = true;
      } catch (VariableNotFoundError &e) {
        // Variable not found, add it to the current scope
      }

      if (!is_defined)
        node = static_pointer_cast<FuncDefNode>(
            remove_variable(node, scope_stack.back(), it->first));
    }
  }

  scope_stack.pop_back();

  // Update function in functions map
#ifdef DEBUG
  cout << "updating function " << identifier << endl;
#endif
  scope_stack.back().variables[identifier] = node;

  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_funccall(shared_ptr<FuncCallNode> node) {
  string identifier = node->getName()->value;
  Scope &scope = scope_stack.back();

#ifdef DEBUG
  cout << "analyze_funccall " << identifier << endl;
#endif

  try {
    shared_ptr<ASTNode> func_node = find_function(node->getName());

    if (func_node->node_type == FUNCDEF) {
      vector<shared_ptr<ASTNode>> args;

      for (auto &child : node->children) {
        args.push_back(analyze_node(child));
      }

      return get_inlined_function(node->getName(), args);
    }

  } catch (FunctionNotFoundError &e) {

    if (identifier == "require") {
      inline_require(node);
      return nullptr;
    }

    // Function not found, check if it is a predefined function
    if (find(PF_FUNCTIONS.begin(), PF_FUNCTIONS.end(), identifier) ==
        PF_FUNCTIONS.end())
      throw FunctionNotFoundError(node->head->span, identifier);
    else {
      if (identifier == "plus" || identifier == "minus" ||
          identifier == "times" || identifier == "divide") {
        return calculate_node(node);
      }
    }
  }

  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_lambda(shared_ptr<LambdaNode> node) {
#ifdef DEBUG
  cout << "analyze_lambda" << endl;
#endif
  Scope &scope = scope_stack.back();

  // create new scope for function
  scope_stack.push_back(Scope(node));

  // add parameters to function scope
  for (auto &child : node->getParams()->children) {
    string identifier = child->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

#ifdef DEBUG
    cout << "adding variable " << identifier << endl;
#endif
    scope.variables[identifier] = child;
  }

  node->setBody(analyze_node(node->getBody()));

  // remove variables that has 0 referrers
  for (auto it = scope_stack.back().variables.rbegin();
       it != scope_stack.back().variables.rend(); ++it) {
    if (it->second.referrers == 0) {
#ifdef DEBUG
      cout << "removing variable " << it->first << endl;
#endif
      bool is_defined = false;

      try {
        find_variable(
            make_shared<Token>(IDENTIFIER, it->first, node->head->span));
        is_defined = true;
      } catch (VariableNotFoundError &e) {
        // Variable not found, add it to the current scope
      }

      if (!is_defined)
        node = static_pointer_cast<LambdaNode>(
            remove_variable(node, scope_stack.back(), it->first));
    }
  }

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_prog(shared_ptr<ProgNode> node) {
#ifdef DEBUG
  cout << "analyze_prog" << endl;
#endif
  scope_stack.push_back(Scope(node));

  for (auto &local : node->getLocals()->children) {
    string identifier = local->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

#ifdef DEBUG
    cout << "adding variable " << identifier << endl;
#endif
    scope.variables[identifier] = local;
  }

  // start from 1 to skip locals
  for (auto it = node->children.begin() + 1; it != node->children.end(); ++it) {
    *it = analyze_node(*it);
  }

  // remove variables that has 0 referrers and inlined functions
  for (auto it = scope_stack.back().variables.rbegin();
       it != scope_stack.back().variables.rend(); ++it) {
    if (it->second.referrers == 0) {
#ifdef DEBUG
      cout << "removing variable " << it->first << endl;
#endif
      bool is_defined = false;

      try {
        find_variable(
            make_shared<Token>(IDENTIFIER, it->first, node->head->span));
        is_defined = true;
      } catch (VariableNotFoundError &e) {
        // Variable not found, add it to the current scope
      }

      if (!is_defined)
        node = static_pointer_cast<ProgNode>(
            remove_variable(node, scope_stack.back(), it->first));
    } else if (it->second.value->node_type == FUNCDEF) {
#ifdef DEBUG
      cout << "removing function " << it->first << endl;
#endif
      node = static_pointer_cast<ProgNode>(
          remove_inlined_function(node, scope_stack.back(), it->first));
    }
  }

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_setq(shared_ptr<SetqNode> node) {
#ifdef DEBUG
  cout << "analyze_setq" << endl;
#endif
  string identifier = node->getName()->value;
  Scope &scope = scope_stack.back();

  try {
    find_variable(node->getName());
  } catch (VariableNotFoundError &e) {
    // Variable not found, add it to the current scope
  }

#ifdef DEBUG
  cout << "successfully added variable " << identifier << endl;
#endif
  scope.variables[identifier] = node;

  node->setValue(analyze_node(node->getValue()));
  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_return(shared_ptr<ReturnNode> node) {
#ifdef DEBUG
  cout << "analyze_return" << endl;
#endif

  node->setValue(analyze_node(node->getValue()));

  // check if return is allowed in current scope
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    switch (it->scope_node->node_type) {
    case FUNCDEF:
      return node;
    case LAMBDA:
      return node;
    case PROG:
      return node;
    default:
      break;
    }
  }

  throw RuntimeError(node->head->span, "Return is not allowed here");
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_break(shared_ptr<ASTNode> node) {
#ifdef DEBUG
  cout << "analyze_break" << endl;
#endif
  // check if break is allowed in current scope
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    switch (it->scope_node->node_type) {
    case WHILE:
      return node;
    default:
      break;
    }
  }

  throw RuntimeError(node->head->span, "Break is not allowed here");
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_while(shared_ptr<WhileNode> node) {

#ifdef DEBUG
  cout << "analyze_while" << endl;
#endif
  scope_stack.push_back(Scope(node));

  node->setCond(analyze_node(node->getCond()));
  node->setBody(analyze_node(node->getBody()));

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_cond(shared_ptr<CondNode> node) {
#ifdef DEBUG
  cout << "analyze_cond" << endl;
#endif
  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::calculate_node(shared_ptr<ASTNode> node) {
#ifdef DEBUG
  cout << "calculate_node" << endl;
#endif
  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  vector<shared_ptr<ASTNode>> args; // calculable arguments
  vector<shared_ptr<ASTNode>> left; // non-calculable arguments

  for (auto &child : node->children) {
    if (child->calculable())
      args.push_back(child);
    else
      left.push_back(child);
  }

  if (left.size() == 0) { // all arguments are calculable
    return make_shared<ASTNode>(LEAF, calculate(args, node->head->value));
  } else if (args.size() == 0) { // no arguments are calculable
    return make_shared<FuncCallNode>(node->head, left);
  } else { // calculate calculable arguments and create new function call node
    if (node->children[0]->calculable()) {
      left.insert(left.begin(), make_shared<ASTNode>(
                                    LEAF, calculate(args, node->head->value)));
    } else {
      left.push_back(
          make_shared<ASTNode>(LEAF, calculate(args, node->head->value)));
    }
    return make_shared<FuncCallNode>(node->head, left);
  }
}
