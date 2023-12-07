CC := g++
CFLAGS := -O2 -ly -ll
GRAPHVIZ_LIBS := -lgvc -lcgraph -lcdt -I/usr/include/graphviz
OBJS := main.o interpreter.o pf_funcs.o utils.o semantic_analyzer.o scanner.o parser.tab.o driver.o ast.o
TARGET := flang_repl

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(GRAPHVIZ_LIBS)

main.o: main.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

interpreter.o: interpreter.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

pf_funcs.o: pf_funcs.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

utils.o: utils.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

semantic_analyzer.o: semantic_analyzer.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

scanner.o: scanner.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

parser.tab.o: parser.tab.cc
	$(CC) -c -o $@ $< $(CFLAGS)

driver.o: driver.cc
	$(CC) -c -o $@ $< $(CFLAGS)

ast.o: ast.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) parser.tab.cc parser.tab.hh location.hh position.hh stack.hh scanner.cpp

clean_obj:
	rm -rf $(OBJS)

all:
	bison -d parser.yy
	flex -o scanner.cpp scanner.l
	make
	make clean_obj
	
