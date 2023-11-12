#include "semantic_analyzer.h"
#include "ast.h"
#include <algorithm>
#include <iostream>
#include <memory>

using std::cout, std::endl;

SemanticAnalyzer::SemanticAnalyzer() {}

SemanticAnalyzer::~SemanticAnalyzer() {}

void SemanticAnalyzer::analyze(shared_ptr<ASTNode> &root) {
  scope_stack.push_back(Scope(map<string, Var>(), PROGRAM));

  for (auto &node : root->children) {
    analyze_node(node);
  }
}

Var SemanticAnalyzer::find_variable(shared_ptr<Token> identifier) {
  string identifier_str = identifier->value;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto variable = it->variables.find(identifier_str);
    if (variable != it->variables.end()) {
      variable->second.referrers++;
      return variable->second;
    }
  }

  cout << "error in find " << identifier_str << endl;
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

  cout << "error in remove var" << identifier << endl;
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
          node->children.erase(node->children.begin() + i);
          return node;
        }
      }
    }
  }

  cout << "error in remove func" << identifier << endl;
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

  if (args.size() != params.size())
    throw WrongNumberOfArgumentsError(func_copy->head->span, identifier->value,
                                      params.size(), args.size());

  cout << "get_inlined_function" << endl;
  for (auto &param : params) {
    string tmp = "tmp" + to_string(tmps.size());
    tmps[param->head->value] = tmp;
  }

  for (auto &child : node_body->children) {
    child = inline_function(child, tmps);
  }

  vector<shared_ptr<ASTNode>> setqs;
  setqs.push_back(make_shared<ListNode>());

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

  body_children.push_back(node_body->copy());

  return make_shared<ProgNode>(node_body->head, body_children);
}

shared_ptr<ASTNode>
SemanticAnalyzer::inline_function(shared_ptr<ASTNode> node,
                                  map<string, string> const &args) {
  cout << "inline_function" << endl;

  if (node->node_type == LEAF) {
    if (node->head->type == IDENTIFIER) {
      auto arg = args.find(node->head->value);
      if (arg != args.end()) {
        cout << "replacing " << node->head->value << " with " << arg->second
             << endl;
        return make_shared<ASTNode>(
            LEAF,
            make_shared<Token>(IDENTIFIER, arg->second, node->head->span));
      }
    }
  }

  for (auto &child : node->children) {
    child = inline_function(child, args);
  }

  return node;
}

shared_ptr<FuncDefNode>
SemanticAnalyzer::find_function(shared_ptr<Token> identifier) {
  string identifier_str = identifier->value;

  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto variable = it->variables.find(identifier_str);
    if (variable != it->variables.end()) {
      if (variable->second.value->node_type == FUNCDEF) {
        return static_pointer_cast<FuncDefNode>(variable->second.value);
      }
    }
  }

  throw FunctionNotFoundError(identifier->span, identifier_str);
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_node(shared_ptr<ASTNode> node) {
  cout << "analyze_node " << node->head->value << endl;
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
    cout << "analyze_node " << node->head->value << endl;
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
  Scope &scope = scope_stack.back();

  cout << "analyze_funcdef " << identifier << endl;

  try {
    find_function(node->getName());
    throw FunctionAlreadyDefinedError(node->getName()->span, identifier);
  } catch (FunctionNotFoundError &e) {
    // Function not found, add it to the list of functions
  }

  cout << "adding function " << identifier << endl;
  scope.variables[identifier] = node;

  scope_stack.push_back(Scope(map<string, Var>(), FUNCDEF));

  for (auto &child : node->getParams()->children) {
    string identifier = child->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

    cout << "adding variable " << identifier << endl;
    scope.variables[identifier] = child;
  }

  node->setBody(analyze_node(node->getBody()));

  // remove variables that has 0 referrers
  for (auto it = scope_stack.back().variables.rbegin();
       it != scope_stack.back().variables.rend(); ++it) {
    if (it->second.referrers == 0) {
      cout << "removing variable " << it->first << endl;
      node = static_pointer_cast<FuncDefNode>(
          remove_variable(node, scope_stack.back(), it->first));
    }
  }

  // Update function in functions map
  cout << "updating function " << identifier << endl;
  scope.variables[identifier] = node;

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_funccall(shared_ptr<FuncCallNode> node) {
  string identifier = node->getName()->value;
  Scope &scope = scope_stack.back();

  cout << "analyze_funccall " << identifier << endl;

  try {
    find_function(node->getName());

    vector<shared_ptr<ASTNode>> args;

    for (auto &child : node->children) {
      args.push_back(analyze_node(child));
    }

    return get_inlined_function(node->getName(), args);

  } catch (FunctionNotFoundError &e) {
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
  cout << "analyze_lambda" << endl;
  Scope &scope = scope_stack.back();

  scope_stack.push_back(Scope(map<string, Var>(), LAMBDA));

  for (auto &child : node->getParams()->children) {
    string identifier = child->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

    cout << "adding variable " << identifier << endl;
    scope.variables[identifier] = child;
  }

  node->setBody(analyze_node(node->getBody()));

  for (auto it = scope_stack.back().variables.rbegin();
       it != scope_stack.back().variables.rend(); ++it) {
    if (it->second.referrers == 0) {
      cout << "removing variable " << it->first << endl;
      node = static_pointer_cast<LambdaNode>(
          remove_variable(node, scope_stack.back(), it->first));
    }
  }

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_prog(shared_ptr<ProgNode> node) {
  cout << "analyze_prog" << endl;
  scope_stack.push_back(Scope(map<string, Var>(), PROG));

  for (auto &local : node->getLocals()->children) {
    string identifier = local->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

    cout << "adding variable " << identifier << endl;
    scope.variables[identifier] = local;
  }

  for (auto it = node->children.begin() + 1; it != node->children.end(); ++it) {
    *it = analyze_node(*it);
  }

  for (auto it = scope_stack.back().variables.rbegin();
       it != scope_stack.back().variables.rend(); ++it) {
    if (it->second.referrers == 0) {
      cout << "removing variable " << it->first << endl;
      node = static_pointer_cast<ProgNode>(
          remove_variable(node, scope_stack.back(), it->first));
    } else if (it->second.value->node_type == FUNCDEF) {
      cout << "removing function " << it->first << endl;
      node = static_pointer_cast<ProgNode>(
          remove_inlined_function(node, scope_stack.back(), it->first));
    }
  }

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_setq(shared_ptr<SetqNode> node) {
  cout << "analyze_setq" << endl;
  string identifier = node->getName()->value;
  Scope &scope = scope_stack.back();

  try {
    find_variable(node->getName());
  } catch (VariableNotFoundError &e) {
    // Variable not found, add it to the current scope
  }

  cout << "successfully added variable " << identifier << endl;
  scope.variables[identifier] = node;

  node->setValue(analyze_node(node->getValue()));
  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_return(shared_ptr<ReturnNode> node) {
  cout << "analyze_return" << endl;

  node->setValue(analyze_node(node->getValue()));

  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    switch (it->scope_type) {
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
  cout << "analyze_break" << endl;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    switch (it->scope_type) {
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

  cout << "analyze_while" << endl;
  scope_stack.push_back(Scope(map<string, Var>(), WHILE));

  node->setCond(analyze_node(node->getCond()));
  node->setBody(analyze_node(node->getBody()));

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_cond(shared_ptr<CondNode> node) {
  cout << "analyze_cond" << endl;
  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::calculate_node(shared_ptr<ASTNode> node) {
  cout << "calculate_node" << endl;
  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  vector<shared_ptr<ASTNode>> args;
  vector<shared_ptr<ASTNode>> left;

  for (auto &child : node->children) {
    if (child->calculable())
      args.push_back(child);
    else
      left.push_back(child);
  }

  if (left.size() == 0) {
    return make_shared<ASTNode>(LEAF, calculate(args, node->head->value));
  } else if (args.size() == 0) {
    return make_shared<FuncCallNode>(node->head, left);
  } else {
    left.push_back(
        make_shared<ASTNode>(LEAF, calculate(args, node->head->value)));
    return make_shared<FuncCallNode>(node->head, left);
  }
}
