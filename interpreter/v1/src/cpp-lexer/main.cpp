#include "ast.h"
#include "driver.hh"
#include "interpeter.h"
#include "semantic_analyzer.h"
#include <graphviz/gvc.h>
#include <iostream>

using interp::Interpreter;

void generate_graph_svg(std::shared_ptr<flang::ASTNode> const &ast,
                        std::string const &filename = "ast.svg") {

  if (ast == nullptr)
    return;

  GVC_t *gvc = gvContext();

  std::shared_ptr<Agraph_t> ast_graph =
      std::shared_ptr<Agraph_t>(agopen((char *)"ast", Agdirected, NULL));

  ast->print(ast_graph);

  FILE *astdot = fopen((char *)"ast.dot", (char *)"w");
  agwrite(ast_graph.get(), astdot);

  fclose(astdot);
  gvFreeContext(gvc);

  std::string cmd = "dot -Tsvg ast.dot > " + filename;
  system(cmd.c_str());
}

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
          drv.parse("tmp.flang");
          semantic_analyzer.analyze(drv.ast);
          generate_graph_svg(drv.ast, "repl.svg");
          interpreter.interpret(drv.ast);
        } catch (std::exception &e) {
          std::cout << e.what() << '\n';
          semantic_analyzer.clear_stack(drv.ast);
          drv.clear_ast();
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
