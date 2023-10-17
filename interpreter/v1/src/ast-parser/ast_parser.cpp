#include <iostream>
#include <vector>

#include "astnode.h"
#include "parse-node-funcs.h"
#include "token.h"

using namespace std;

int main() {
  vector<Token> tokens;

  parseTokens(tokens);

  vector<any> ast = parseTokensToVector(tokens);

  ASTNode *root = parseVectorToASTNode(ast);

  cout << "AST:" << endl;
  root->print();

  return 0;
}
