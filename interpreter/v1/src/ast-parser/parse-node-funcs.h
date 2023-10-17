#pragma once
#include <any>
#include <tuple>
#include <vector>

#include "astnode.h"
#include "token.h"

ASTNode *parseASTNode(vector<any> ast);
ListASTNode *parseListASTNode(vector<any> ast);
FuncDeclASTNode *parseFuncDeclASTNode(vector<any> ast);
SetqASTNode *parseSetqASTNode(vector<any> ast);
WhileASTNode *parseWhileASTNode(vector<any> ast);
LambdaASTNode *parseLambdaASTNode(vector<any> ast);
tuple<vector<any>, int> parseVector(vector<Token> tokens, int start = 1);
vector<any> parseTokensToVector(vector<Token> tokens);
ASTNode *parseVectorToASTNode(vector<any> ast);
void parseTokens(vector<Token> &tokens);
