#include "semantic_analyzer.h"
#include "ast.h"
#include <algorithm>
#include <iostream>
#include <memory>

using namespace std;
using namespace flang;

SemanticAnalyzer::SemanticAnalyzer() {}

SemanticAnalyzer::~SemanticAnalyzer() {}

void SemanticAnalyzer::analyze(const shared_ptr<ASTNode> &root) {
  scope_stack.push_back(Scope(map<string, shared_ptr<ASTNode>>(), PROGRAM));

  for (auto &node : root->children) {
    analyze_node(node);
  }
}

shared_ptr<ASTNode>
SemanticAnalyzer::find_variable(const shared_ptr<Token> &identifier) {
  string identifier_str = identifier->value;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto variable = it->variables.find(identifier_str);
    if (variable != it->variables.end())
      return variable->second;
  }

  throw VariableNotFoundError(identifier->span, identifier_str);
}

shared_ptr<ASTNode>
SemanticAnalyzer::find_function(const shared_ptr<Token> &identifier,
                                bool return_body) {
  string identifier_str = identifier->value;
  auto function = functions.find(identifier_str);
  if (function != functions.end())
    return return_body ? function->second->body : function->second;

  throw FunctionNotFoundError(identifier->span, identifier_str);
}

void SemanticAnalyzer::analyze_node(const shared_ptr<ASTNode> &node) {
  switch (node->node_type) {
  case FUNCDEF:
    analyze_funcdef(static_pointer_cast<FuncDefNode>(node));
    return;
  case FUNCCALL:
    analyze_funccall(static_pointer_cast<FuncCallNode>(node));
    return;
  case LAMBDA:
    analyze_lambda(static_pointer_cast<LambdaNode>(node));
    return;
  case PROG:
    analyze_prog(static_pointer_cast<ProgNode>(node));
    return;
  case SETQ:
    analyze_setq(static_pointer_cast<SetqNode>(node));
    return;
  case RETURN:
    analyze_return(static_pointer_cast<ReturnNode>(node));
    return;
  case WHILE:
    analyze_while(static_pointer_cast<WhileNode>(node));
    return;
  case COND:
    analyze_cond(static_pointer_cast<CondNode>(node));
    return;
  case BREAK:
    analyze_break(node);
    return;
  default:
    break;
  }

  if (node->head != nullptr && node->head->type == IDENTIFIER) {
    cout << "analyze_node " << node->head->value << endl;
    find_variable(node->head);
  }

  for (auto &child : node->children) {
    analyze_node(child);
  }
}

void SemanticAnalyzer::analyze_funcdef(const shared_ptr<FuncDefNode> &node) {
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

  analyze_node(node->body);
  scope_stack.pop_back();
}

void SemanticAnalyzer::analyze_funccall(const shared_ptr<FuncCallNode> &node) {
  string identifier = node->name->value;
  Scope &scope = scope_stack.back();

  cout << "analyze_funccall " << identifier << endl;

  try {
    find_function(node->name);
  } catch (RuntimeError &e) {
    // Function not found, check if it is a primitive function
    if (find(PF_FUNCTIONS.begin(), PF_FUNCTIONS.end(), identifier) ==
        PF_FUNCTIONS.end())
      throw FunctionNotFoundError(node->head->span, identifier);
  }

  for (auto &child : node->children) {
    analyze_node(child);
  }
}

void SemanticAnalyzer::analyze_lambda(const shared_ptr<LambdaNode> &node) {
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

  analyze_node(node->body);
  scope_stack.pop_back();
}

void SemanticAnalyzer::analyze_prog(const shared_ptr<ProgNode> &node) {
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
    analyze_node(*it);
  }

  scope_stack.pop_back();
}

void SemanticAnalyzer::analyze_setq(const shared_ptr<SetqNode> &node) {
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

  analyze_node(node->value);
}

void SemanticAnalyzer::analyze_return(const shared_ptr<ReturnNode> &node) {
  cout << "analyze_return" << endl;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    switch (it->scope_type) {
    case FUNCDEF:
      return;
    case LAMBDA:
      return;
    case PROG:
      return;
    default:
      break;
    }
  }

  throw RuntimeError(node->head->span, "Return is not allowed here");
}

void SemanticAnalyzer::analyze_break(const shared_ptr<ASTNode> &node) {
  cout << "analyze_break" << endl;
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    switch (it->scope_type) {
    case WHILE:
      return;
    default:
      break;
    }
  }

  throw RuntimeError(node->head->span, "Break is not allowed here");
}

void SemanticAnalyzer::analyze_while(const shared_ptr<WhileNode> &node) {

  cout << "analyze_while" << endl;
  analyze_node(node->cond);
  analyze_node(node->body);
}

void SemanticAnalyzer::analyze_cond(const shared_ptr<CondNode> &node) {
  cout << "analyze_cond" << endl;
  for (auto &child : node->children) {
    analyze_node(child);
  }
}
