CC := g++
CFLAGS := -O2 -ly -ll
GRAPHVIZ_LIBS := -lgvc -lcgraph -lcdt -I/usr/include/graphviz
OBJS := obj/main.o obj/interpreter.o obj/pf_funcs.o obj/utils.o obj/semantic_analyzer.o obj/scanner.o obj/parser.tab.o obj/driver.o obj/ast.o
TARGET := flang_repl

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(GRAPHVIZ_LIBS)

obj/main.o: main.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

obj/interpreter.o: interpreter/interpreter.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

obj/pf_funcs.o: utils/pf_funcs.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

obj/utils.o: utils/utils.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

obj/semantic_analyzer.o: semantic/semantic_analyzer.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

obj/scanner.o: parser/scanner.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

obj/parser.tab.o: parser/parser.tab.cc
	$(CC) -c -o $@ $< $(CFLAGS)

obj/driver.o: parser/driver.cc
	$(CC) -c -o $@ $< $(CFLAGS)

obj/ast.o: parser/ast.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(TARGET) parser/parser.tab.cc parser/parser.tab.hh parser/location.hh parser/position.hh parser/stack.hh parser/scanner.cpp

clean_obj:
	rm -rf obj

all:

	cd parser && bison -d parser.yy && flex -o scanner.cpp scanner.l
	mkdir -p obj
	make
	make clean_obj
	
