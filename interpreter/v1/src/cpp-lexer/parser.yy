%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.1"
%header

%define api.token.constructor
%define api.value.type variant
%define parse.assert


%code requires {
  #include <string>
  #include <iostream>
  #include <memory>
  #include <graphviz/gvc.h>
  #include "ast.h"
  #define NO_LAYOUT_OR_RENDERING
  class Driver;

}

%param { Driver& driver }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
  #include "driver.hh"

}


%define api.token.prefix {TOK_}
%token <std::string> SF_BREAK SF_COND SF_FUNC SF_LAMBDA SF_PROG SF_QUOTE SF_RETURN SF_SETQ SF_WHILE
%token <std::string> PF_PLUS PF_TIMES PF_DIVIDE PF_MINUS
%token <std::string> PF_HEAD PF_TAIL PF_CONS
%token <std::string> PF_EQUAL PF_NONEQUAL PF_LESS PF_LESSEQ PF_GREATER PF_GREATEREQ
%token <std::string> PF_ISINT PF_ISREAL PF_ISBOOL PF_ISNULL PF_ISATOM PF_ISLIST
%token <std::string> PF_AND PF_OR PF_XOR PF_NOT
%token <std::string> PF_EVAL
%token <std::string> IDENTIFIER
%token <std::string> INT
%token <std::string> REAL
%token <std::string> TRUE FALSE
%token <std::string> NULL
%token <std::string> SYM_SPACE SYM_TAB EOL 
%token <std::string> SYM_LPAREN "(" 
%token <std::string> SYM_RPAREN ")" 


%type <std::string> del
%type <std::string> d

%type <std::shared_ptr<flang::FuncDefNode>> func_def
%type <std::shared_ptr<flang::LambdaNode>> lambda_def
%type <std::shared_ptr<flang::ListNode>> quote_def
%type <std::shared_ptr<flang::ReturnNode>> return_def
%type <std::shared_ptr<flang::ASTNode>> break_def
%type <std::shared_ptr<flang::WhileNode>> while_def
%type <std::shared_ptr<flang::SetqNode>> setq_def

%type <std::shared_ptr<flang::FuncCallNode>> plus_def
%type <std::shared_ptr<flang::FuncCallNode>> times_def
%type <std::shared_ptr<flang::FuncCallNode>> divide_def
%type <std::shared_ptr<flang::FuncCallNode>> minus_def
%type <std::shared_ptr<flang::FuncCallNode>> head_def
%type <std::shared_ptr<flang::FuncCallNode>> tail_def
%type <std::shared_ptr<flang::FuncCallNode>> cons_def
%type <std::shared_ptr<flang::FuncCallNode>> not_def
%type <std::shared_ptr<flang::FuncCallNode>> func_call

%type <std::shared_ptr<flang::ASTNode>> program
%type <std::shared_ptr<flang::ASTNode>> element
%type <std::shared_ptr<flang::ASTNode>> stmt 
%type <std::vector<std::shared_ptr<flang::ASTNode>>> elements
%type <std::shared_ptr<flang::ListNode>> list

%type <std::shared_ptr<flang::Token>> atom
%type <std::shared_ptr<flang::Token>> literal

%%

program:
    elements
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, "program");
      $$ = std::make_shared<flang::ASTNode>(flang::ASTNodeType::PROGRAM, t, $1);

    #ifdef NO_LAYOUT_OR_RENDERING

      GVC_t *gvc = gvContext();

    #endif

      std::shared_ptr<Agraph_t> ast = std::shared_ptr<Agraph_t>(agopen((char*)"ast", Agdirected, NULL));

      $$->print(ast);
      
    #ifdef NO_LAYOUT_OR_RENDERING

      FILE *astdot = fopen((char*)"ast.dot", (char*)"w");
      agwrite(ast.get(), astdot);

    #else

      gvLayout(gvc, ast.get(), (char*)"dot");
      gvRender(gvc, ast.get(), (char*)"png", fopen((char*)"ast.png", (char*)"w"));
      gvFreeLayout(gvc, ast.get());

    #endif

      agclose(ast.get());
      fclose(astdot);
      gvFreeContext(gvc);

      system("dot -Tsvg ast.dot > ast.svg");
    }

elements:
  { $$ = std::vector<std::shared_ptr<flang::ASTNode>>();}
  | element del elements
  {
    std::vector<std::shared_ptr<flang::ASTNode>> v;

    v.push_back($1);

    for (auto& e : $3) {
      v.push_back(e);
    }

    $$ = v;
  }


element:
      atom
      {
        $$ = std::make_shared<flang::ASTNode>(flang::ASTNodeType::LEAF, $1);
      }
    | literal 
      {
        $$ = std::make_shared<flang::ASTNode>(flang::ASTNodeType::LEAF, $1);
      }
    | stmt { $$ = $1;}
    | list { $$ = $1;}

pf_func:
    element 
  | PF_PLUS | PF_TIMES | PF_DIVIDE | PF_MINUS
  | PF_HEAD | PF_TAIL | PF_CONS
  | PF_EQUAL | PF_NONEQUAL | PF_LESS | PF_LESSEQ | PF_GREATER | PF_GREATEREQ
  | PF_ISINT | PF_ISREAL | PF_ISBOOL | PF_ISNULL | PF_ISATOM | PF_ISLIST
  | PF_AND | PF_OR | PF_XOR | PF_NOT
  | PF_EVAL
  ;

pf_funcs:
  | pf_func del pf_funcs

func_def:
    "(" del SF_FUNC d element d list d list del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7,
        $9
      };

      $$ = std::make_shared<flang::FuncDefNode>(t, children);
    }

lambda_def:
    "(" del SF_LAMBDA d list d list del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::LambdaNode>(t, children);
    }

quote_def:
    "(" del SF_QUOTE d elements del ")"
    {
      $$ = std::make_shared<flang::ListNode>($5); 
    }

return_def:
    "(" del SF_RETURN d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5
      };

      $$ = std::make_shared<flang::ReturnNode>(t, children);
    }

break_def:
    "(" del SF_BREAK del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      $$ = std::make_shared<flang::ASTNode>(flang::ASTNodeType::BREAK, t);
    }

while_def:
    "(" del SF_WHILE d element d list del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::WhileNode>(t, children);
    }

plus_def:
    "(" del PF_PLUS d element d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

times_def:
    "(" del PF_TIMES d element d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

divide_def:
    "(" del PF_DIVIDE d element d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

minus_def:
    "(" del PF_MINUS d element d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

head_def:
    "(" del PF_HEAD d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

tail_def:
    "(" del PF_TAIL d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

cons_def:
    "(" del PF_CONS d element d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

setq_def:
    "(" del SF_SETQ d element d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::IDENTIFIER, $3);
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5,
        $7
      };

      $$ = std::make_shared<flang::SetqNode>(t, children);
    }

not_def:
    "(" del PF_NOT d element del ")"
    {
      std::shared_ptr<flang::Token> t = std::make_shared<flang::Token>(flang::TokenType::KEYWORD, "not");
      std::vector<std::shared_ptr<flang::ASTNode>> children = {
        $5
      };

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

func_call:
    "(" del atom d elements del ")"
    {
      std::shared_ptr<flang::Token> t = $3; 
      std::vector<std::shared_ptr<flang::ASTNode>> children = $5;

      $$ = std::make_shared<flang::FuncCallNode>(t, children);
    }

list:
  "(" del elements del ")" { $$ = std::make_shared<flang::ListNode>($3); }

stmt:
  | func_def {$$ = $1;}
  | lambda_def {$$ = $1;}
  | quote_def {$$ = $1;}
  | return_def {$$ = $1;}
  | break_def {$$ = $1;}
  | while_def {$$ = $1;}
  | plus_def {$$ = $1;}
  | times_def {$$ = $1;}
  | divide_def {$$ = $1;}
  | minus_def {$$ = $1;}
  | head_def {$$ = $1;}
  | tail_def {$$ = $1;}
  | cons_def {$$ = $1;}
  | setq_def {$$ = $1;}
  | not_def {$$ = $1;}
  | func_call {$$ = $1;}
  ;


atom:
  IDENTIFIER
  {
    $$ = std::make_shared<flang::Token>(flang::TokenType::IDENTIFIER, $1);
  }

literal:
  INT { $$ = std::make_shared<flang::Token>(flang::TokenType::INT, $1); }
  | REAL { $$ = std::make_shared<flang::Token>(flang::TokenType::REAL, $1); } 
  | TRUE { $$ = std::make_shared<flang::Token>(flang::TokenType::BOOL, $1); } 
  | FALSE { $$ = std::make_shared<flang::Token>(flang::TokenType::BOOL, $1); }
  | NULL { $$ = std::make_shared<flang::Token>(flang::TokenType::NUL, $1); }

del:
  { $$ = "";}
  | d del

d:
  SYM_SPACE | SYM_TAB | EOL


%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << std::endl;
}
