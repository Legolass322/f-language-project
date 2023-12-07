#include "utils.h"

void generate_graph_svg(std::shared_ptr<flang::ASTNode> const &ast,
                        std::string const &filename) {

  if (ast == nullptr)
    return;

  GVC_t *gvc = gvContext();

  std::shared_ptr<Agraph_t> ast_graph =
      std::shared_ptr<Agraph_t>(agopen((char *)"ast", Agdirected, NULL));

  ast->print(ast_graph);

  FILE *astdot = fopen((char *)"svg/ast.dot", (char *)"w");
  agwrite(ast_graph.get(), astdot);

  fclose(astdot);
  gvFreeContext(gvc);

  std::string cmd = "dot -Tsvg svg/ast.dot > svg/" + filename;
  system(cmd.c_str());
}
