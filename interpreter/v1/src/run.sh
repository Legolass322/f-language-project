make
g++ -o ast_parser ast_parser.cpp
cat input.txt | ./f | ./ast_parser
