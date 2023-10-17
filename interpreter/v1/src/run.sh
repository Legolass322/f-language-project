make
g++ -o ast_parser ./ast-parser/ast_parser.cpp ./ast-parser/astnode.cpp ./ast-parser/parse-node-funcs.cpp
cat input.txt | ./f | ./ast_parser
