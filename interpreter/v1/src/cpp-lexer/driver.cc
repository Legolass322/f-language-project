#include "driver.hh"

Driver::Driver() : trace_scanning(false), trace_parsing(false) {}

int Driver::parse(const std::string &f) {
  file = f;
  this->ast = nullptr;
  scan_begin();
  yy::parser parser(*this);
  parser.set_debug_level(trace_parsing);
  int res = parser.parse();
  scan_end();
  return res;
}

void Driver::parse_ast(const std::shared_ptr<flang::ASTNode> &ast) {
  this->ast = ast;
}

void Driver::clear_ast() { this->ast = nullptr; }
