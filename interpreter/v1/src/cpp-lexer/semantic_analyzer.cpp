#include "semantic_analyzer.h"
#include "ast.h"
#include <algorithm>
#include <iostream>
#include <memory>

using namespace std;
using namespace flang;

SemanticAnalyzer::SemanticAnalyzer() {}

SemanticAnalyzer::~SemanticAnalyzer() {}

void SemanticAnalyzer::analyze(shared_ptr<ASTNode> &root) {
  scope_stack.push_back(Scope(map<string, shared_ptr<ASTNode>>(), PROGRAM));

  for (auto &node : root->children) {
    analyze_node(node);
  }
}

shared_ptr<ASTNode>
SemanticAnalyzer::find_variable(shared_ptr<Token> identifier) {
  string identifier_str = identifier->value;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto variable = it->variables.find(identifier_str);
    if (variable != it->variables.end())
      return variable->second;
  }

  throw VariableNotFoundError(identifier->span, identifier_str);
}

shared_ptr<ASTNode>
SemanticAnalyzer::find_function(shared_ptr<Token> identifier) {
  string identifier_str = identifier->value;
  auto function = functions.find(identifier_str);
  if (function != functions.end())
    return function->second;

  throw FunctionNotFoundError(identifier->span, identifier_str);
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_node(shared_ptr<ASTNode> node) {
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
  }

  for (auto &child : node->children) {
    child = analyze_node(child);
  }

  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_funcdef(shared_ptr<FuncDefNode> node) {
  string identifier = node->name->value;
  Scope &scope = scope_stack.back();

  cout << "analyze_funcdef " << identifier << endl;

  try {
    find_function(node->name);
    throw FunctionAlreadyDefinedError(node->name->span, identifier);
  } catch (FunctionNotFoundError &e) {
    // Function not found, add it to the list of functions
  }

  cout << "adding function " << identifier << endl;
  scope.variables[identifier] = node;
  functions[identifier] = node;

  scope_stack.push_back(Scope(map<string, shared_ptr<ASTNode>>(), FUNCDEF));

  for (auto &child : node->params->children) {
    string identifier = child->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

    cout << "adding variable " << identifier << endl;
    scope.variables[identifier] = child;
  }

  node->body = analyze_node(node->body);
  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_funccall(shared_ptr<FuncCallNode> node) {
  string identifier = node->name->value;
  Scope &scope = scope_stack.back();

  cout << "analyze_funccall " << identifier << endl;

  try {
    find_function(node->name);
  } catch (RuntimeError &e) {
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

  scope_stack.push_back(Scope(map<string, shared_ptr<ASTNode>>(), LAMBDA));

  for (auto &child : node->params->children) {
    string identifier = child->head->value;
    Scope &scope = scope_stack.back();
    auto variable = scope.variables.find(identifier);

    // Check if variable is already defined in current scope
    if (variable != scope.variables.end())
      throw VariableAlreadyDefinedError(node->head->span, identifier);

    cout << "adding variable " << identifier << endl;
    scope.variables[identifier] = child;
  }

  node->body = analyze_node(node->body);
  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_prog(shared_ptr<ProgNode> node) {
  cout << "analyze_prog" << endl;
  scope_stack.push_back(Scope(map<string, shared_ptr<ASTNode>>(), PROG));

  for (auto &local : node->locals->children) {
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

  scope_stack.pop_back();
  return node;
}

shared_ptr<ASTNode> SemanticAnalyzer::analyze_setq(shared_ptr<SetqNode> node) {
  cout << "analyze_setq" << endl;
  string identifier = node->name->value;
  Scope &scope = scope_stack.back();

  try {
    find_variable(node->name);
  } catch (VariableNotFoundError &e) {
    // Variable not found, add it to the current scope
  }

  cout << "successfully added variable " << identifier << endl;
  scope.variables[identifier] = node;

  node->value = analyze_node(node->value);
  return node;
}

shared_ptr<ASTNode>
SemanticAnalyzer::analyze_return(shared_ptr<ReturnNode> node) {
  cout << "analyze_return" << endl;
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
  scope_stack.push_back(Scope(map<string, shared_ptr<ASTNode>>(), WHILE));
  node->cond = analyze_node(node->cond);
  node->body = analyze_node(node->body);
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
    if (calculable(child))
      args.push_back(child);
    else
      left.push_back(child);
  }

  if (left.size() == 0) {
    return make_shared<ASTNode>(LEAF, calculate(args, node->head->value));
  } else {
    left.push_back(
        make_shared<ASTNode>(LEAF, calculate(args, node->head->value)));
    return make_shared<FuncCallNode>(node->head, left);
  }
}
