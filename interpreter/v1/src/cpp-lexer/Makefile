all:
	bison -d parser.yy
	flex -o scanner.cpp scanner.l
	g++ -o flang_repl main.cpp interpreter.cpp pf_funcs.cpp utils.cpp semantic_analyzer.cpp scanner.cpp parser.tab.cc driver.cc ast.cpp -ly -ll -lgvc -lcgraph -lcdt -I/usr/include/graphviz
