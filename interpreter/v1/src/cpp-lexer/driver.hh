#ifndef DRIVER_HH
#define DRIVER_HH
#include "ast.h"
#include "parser.tab.hh"
#include <map>
#include <string>

#define YY_DECL yy::parser::symbol_type yylex(Driver &driver)
YY_DECL;

class Driver {
public:
  Driver();

  std::shared_ptr<flang::ASTNode> ast;

  void parse_ast(const std::shared_ptr<flang::ASTNode> &ast);
  void clear_ast();

  int parse(const std::string &f);

  std::string file;

  bool trace_parsing;
  bool trace_scanning;

  void scan_begin();
  void scan_end();

  yy::location location;
};

#endif // DRIVER_HH
