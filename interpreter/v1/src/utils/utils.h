#ifndef UTILS_H
#define UTILS_H

#include "../parser/ast.h"
#include <memory>

void generate_graph_svg(std::shared_ptr<flang::ASTNode> const &ast,
                        std::string const &filename = "ast.svg");

#endif
