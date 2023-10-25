#include "driver.hh"
#include "parser.tab.hh"

Driver::Driver() : trace_scanning(false), trace_parsing(false) {}

int Driver::parse(const std::string &f) {
  file = f;
  scan_begin();
  yy::parser parser(*this);
  parser.set_debug_level(trace_parsing);
  int res = parser.parse();
  scan_end();
  return res;
}
