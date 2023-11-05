#include "ast.h"
#include "driver.hh"
#include "semantic_analyzer.h"
#include <graphviz/gvc.h>
#include <iostream>

void generate_graph_svg(std::shared_ptr<flang::ASTNode> const &ast) {
  GVC_t *gvc = gvContext();

  std::shared_ptr<Agraph_t> ast_graph =
      std::shared_ptr<Agraph_t>(agopen((char *)"ast", Agdirected, NULL));

  ast->print(ast_graph);

  FILE *astdot = fopen((char *)"ast.dot", (char *)"w");
  agwrite(ast_graph.get(), astdot);

  fclose(astdot);
  gvFreeContext(gvc);

  system("dot -Tsvg ast.dot > ast.svg");
}

int main(int argc, char *argv[]) {
  int res = 0;
  Driver drv;
  SemanticAnalyzer semantic_analyzer;

  for (int i = 1; i < argc; ++i) {
    if (argv[i] == std::string("-p"))
      drv.trace_parsing = true;
    else if (argv[i] == std::string("-s"))
      drv.trace_scanning = true;
    else if (!drv.parse(argv[i])) {
      std::cout << "Parsing successful" << '\n';
      semantic_analyzer.analyze(drv.ast);
      std::cout << "Semantic analysis successful" << '\n';
      generate_graph_svg(drv.ast);
      std::cout << "Graphviz file generated" << '\n';
      std::cout << "Done" << '\n';
    } else
      res = 1;
  }
  return res;
}
