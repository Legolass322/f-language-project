#include "ast.h"
#include "driver.hh"
#include "interpeter.h"
#include "semantic_analyzer.h"
#include "utils.h"
#include <graphviz/gvc.h>
#include <iostream>

using interp::Interpreter;

int main(int argc, char *argv[]) {
  int res = 0;
  Driver drv;
  SemanticAnalyzer semantic_analyzer;
  Interpreter interpreter;

  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string("-p"))
      drv.trace_parsing = true;
    else if (argv[i] == std::string("-s"))
      drv.trace_scanning = true;
    else if (argv[i] == std::string("-repl")) {
      std::cout << "Flang REPL 0.0.1" << '\n';
      std::cout << "Type \"exit\" to exit" << '\n';

      while (true) {
        std::cout << ">>> ";
        std::string input;
        std::string line;

        while (getline(std::cin, line)) {
          if (line.empty())
            break;
          input += line + '\n';
        }

        if (input == "exit\n") {
          std::cout << "Bye" << '\n';
          remove("tmp.flang");
          return 0;
        }

        if (input == "")
          continue;

        FILE *tmp = fopen("tmp.flang", "w");
        fprintf(tmp, "%s", input.c_str());
        fclose(tmp);

        try {
          if (drv.parse("tmp.flang")) {
            throw std::runtime_error("Parsing failed");
          }

          semantic_analyzer.analyze(drv.ast);
          generate_graph_svg(drv.ast, "repl.svg");
          interpreter.interpret(drv.ast);
        } catch (std::exception &e) {
          std::cout << e.what() << '\n';
          semantic_analyzer.clear_stack(drv.ast);
        }

        std::cout << '\n';
      }
    } else if (!drv.parse(argv[i])) {
      std::cout << "Parsing successful" << '\n';
      semantic_analyzer.analyze(drv.ast);
      std::cout << "Semantic analysis successful" << '\n';
      generate_graph_svg(drv.ast);
      std::cout << "Graphviz file generated" << '\n';
      interpreter.interpret(drv.ast);
      std::cout << '\n';
      std::cout << "Done" << '\n';
    } else
      res = 1;
  }
  return res;
}
