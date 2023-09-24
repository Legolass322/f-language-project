LEX=flex
YACC=bison
# put together object files from lexer and parser, and
# link the yacc and lex libraries (in that order, to pick
# main() from yacc's library rather than lex's)

f : f.tab.o f.lex.o
	$(CC) -o $@ f.tab.o f.lex.o -ly -ll

# tell make which files yacc will generate
#
# an explanation of the arguments:
# -b roman  -  name the files roman.tab.*
# -d        -  generate a .tab.h file too

f.tab.h f.tab.c : f.y
	$(YACC) -d -b f $?

# the object file relies on the generated lexer, and
# on the token constants 

f.lex.o : f.tab.h f.lex.c

# can't use the default suffix rule because we're
# changing the name of the output to .lex.c

f.lex.c : f.l
	$(LEX) -t $? > $@