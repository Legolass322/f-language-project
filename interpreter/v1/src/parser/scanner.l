%{
  #include <cerrno>
  #include <climits>
  #include <cstdlib>
  #include <cstring> // strerror
  #include "driver.hh"
  #include "parser.tab.hh"
%}

integer ([+|-]?[0-9][0-9]+)
real ([+|-]?[0-9][0-9]+\.[0-9][0-9]+)


letter ([a-zA-Z])
identifier (letter)(letter | digit)+
blank ([ \t\n])

%option yylineno

%{
  #define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  yy::location& loc = driver.location;
  loc.step();
%}

break {return yy::parser::make_SF_BREAK(yytext, loc);}
cond {return yy::parser::make_SF_COND(yytext, loc);}
func {return yy::parser::make_SF_FUNC(yytext, loc);}
lambda {return yy::parser::make_SF_LAMBDA(yytext, loc);}
prog {return yy::parser::make_SF_PROG(yytext, loc);}
quote {return yy::parser::make_SF_QUOTE(yytext, loc);}
return {return yy::parser::make_SF_RETURN(yytext, loc);}
setq {return yy::parser::make_SF_SETQ(yytext, loc);}
while {return yy::parser::make_SF_WHILE(yytext, loc);}

true {return yy::parser::make_TRUE(yytext, loc);}
false {return yy::parser::make_FALSE(yytext, loc);}
null {return yy::parser::make_NULL(yytext, loc);}

\' {return yy::parser::make_SYM_QUOTE(yytext, loc);}
\".+\" {return yy::parser::make_STRING(yytext, loc);}

[+|-]?[0-9]+ {return yy::parser::make_INT (yytext, loc);}
[+|-]?[0-9]+\.[0-9]+ {return yy::parser::make_REAL(yytext, loc);}

[a-zA-Z][a-zA-Z0-9]* {return yy::parser::make_IDENTIFIER(yytext, loc);}

;.+\n {loc.step();}

[ ] {loc.step();}
[\t] {loc.step();}
[\n] {loc.lines(yyleng); loc.step(); }

[\(] {return yy::parser::make_SYM_LPAREN(yytext, loc);}
[\)] {return yy::parser::make_SYM_RPAREN(yytext, loc);}

<<EOF>> {return yy::parser::make_YYEOF(loc);}

. {throw yy::parser::syntax_error (loc, "invalid character: " + std::string(yytext));}
%%

void
Driver::scan_begin ()
{
  YY_FLUSH_BUFFER;
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      std::cerr << "cannot open " << file << ": " << strerror (errno) << '\n';
      exit (EXIT_FAILURE);
    }
}

void
Driver::scan_end ()
{
  fclose (yyin);
}
