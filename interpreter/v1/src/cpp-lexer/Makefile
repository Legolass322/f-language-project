all:
	bison -d parser.yy
	flex -o scanner.cpp scanner.l
	g++ -o cpp_lexer main.cpp scanner.cpp parser.tab.cc driver.cc ast.cpp -ly -ll -lgvc -lcgraph -lcdt -I/usr/include/graphviz
