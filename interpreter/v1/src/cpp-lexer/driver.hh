#ifndef DRIVER_HH
#define DRIVER_HH
#include "parser.tab.hh"
#include <map>
#include <string>

#define YY_DECL yy::parser::symbol_type yylex(Driver &driver)
YY_DECL;

class Driver {
public:
  Driver();

  std::map<std::string, std::string> variables;

  int parse(const std::string &f);

  std::string file;

  bool trace_parsing;
  bool trace_scanning;

  void scan_begin();
  void scan_end();

  yy::location location;
};

#endif // DRIVER_HH
