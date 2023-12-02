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
  #include "ast.h"
  class Driver;

  using namespace flang;

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
%token <std::string> IDENTIFIER
%token <std::string> INT
%token <std::string> REAL
%token <std::string> TRUE FALSE
%token <std::string> NULL
%token <std::string> STRING
%token <std::string> SYM_LPAREN "(" 
%token <std::string> SYM_RPAREN ")" 
%token <std::string> SYM_QUOTE "'"
%token <std::string> SYM_DQUOTE "\""

%left SYM_LPAREN SYM_RPAREN
%left PF_PLUS PF_TIMES PF_DIVIDE PF_MINUS

%type <std::shared_ptr<FuncDefNode>> func_def
%type <std::shared_ptr<LambdaNode>> lambda_def
%type <std::shared_ptr<ASTNode>> quote_def
%type <std::shared_ptr<ReturnNode>> return_def
%type <std::shared_ptr<ASTNode>> break_def
%type <std::shared_ptr<WhileNode>> while_def
%type <std::shared_ptr<SetqNode>> setq_def
%type <std::shared_ptr<ProgNode>> prog_def
%type <std::shared_ptr<CondNode>> cond_def

%type <std::shared_ptr<FuncCallNode>> func_call

%type <std::shared_ptr<ASTNode>> program
%type <std::shared_ptr<ASTNode>> element
%type <std::shared_ptr<ASTNode>> q_element
%type <std::vector<std::shared_ptr<ASTNode>>> elements
%type <std::vector<std::shared_ptr<ASTNode>>> q_elements
%type <std::shared_ptr<ASTNode>> stmt 
%type <std::shared_ptr<ListNode>> list

%type <std::shared_ptr<Token>> atom
%type <std::shared_ptr<Token>> literal
%type <std::shared_ptr<Token>> sf

%%

program:
    elements
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, "program", Span({@1.begin.line, @1.begin.column}));
      $$ = std::make_shared<ASTNode>(ASTNodeType::PROGRAM, t, $1);
      driver.parse_ast($$);

    }

elements:
  { $$ = std::vector<std::shared_ptr<ASTNode>>();}
  | element elements
  {
    std::vector<std::shared_ptr<ASTNode>> v;

    v.push_back($1);

    for (auto& e : $2) {
      v.push_back(e);
    }

    $$ = v;
  }


element:
      atom
      {
        $$ = std::make_shared<ASTNode>(ASTNodeType::LEAF, $1);
      }
    | literal 
      {
        $$ = std::make_shared<ASTNode>(ASTNodeType::LEAF, $1);
      }
    | stmt { $$ = $1;}
    | "(" ")" { $$ = std::make_shared<ListNode>(true, vector<shared_ptr<ASTNode>>());}
    | STRING
    {
      std::string s = $1.substr(1, $1.length() - 2);

      std::vector<std::shared_ptr<ASTNode>> children;

      for (auto& c : s) {
        std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::CHAR, std::string(1, c), Span({@1.begin.line, @1.begin.column}));
        children.push_back(std::make_shared<ASTNode>(ASTNodeType::LEAF, t));
      }

      $$ = std::make_shared<ListNode>(true, children);
    }

func_def:
    "(" SF_FUNC IDENTIFIER list stmt ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<Token> id = std::make_shared<Token>(TokenType::IDENTIFIER, $3, Span({@3.begin.line, @3.begin.column}));
      vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, id),
        $4,
        $5
      };

      $$ = std::make_shared<FuncDefNode>(t, children);
    }

lambda_def:
    "(" SF_LAMBDA list stmt ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      vector<std::shared_ptr<ASTNode>> children = {
        $3,
        $4
      };

      $$ = std::make_shared<LambdaNode>(t, children);
    }

q_elements:
  { $$ = std::vector<std::shared_ptr<ASTNode>>();}
  | q_element q_elements
  {
    std::vector<std::shared_ptr<ASTNode>> v;

    v.push_back($1);

    for (auto& e : $2) {
      v.push_back(e);
    }

    $$ = v;
  }

q_element:
  element { $$ = $1; }
  | sf { $$ = std::make_shared<ASTNode>(ASTNodeType::LEAF, $1); }

quote_def:
    "(" SF_QUOTE q_elements ")"
    {
      $$ = std::make_shared<ListNode>(true, $3); 
    }
    | SYM_QUOTE "(" q_elements ")"
    {
      $$ = std::make_shared<ListNode>(true, $3); 
    }
    | SYM_QUOTE atom
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::CHAR, $2->value, Span({@1.begin.line, @1.begin.column}));
      $$ = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
    }

return_def:
    "(" SF_RETURN element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::vector<std::shared_ptr<ASTNode>> children = {
        $3
      };

      $$ = std::make_shared<ReturnNode>(t, children);
    }

break_def:
    "(" SF_BREAK ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      $$ = std::make_shared<ASTNode>(ASTNodeType::BREAK, t);
    }

while_def:
    "(" SF_WHILE element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      vector<std::shared_ptr<ASTNode>> children = {
        $3,
        $4
      };

      $$ = std::make_shared<WhileNode>(t, children);
    }


setq_def:
    "(" SF_SETQ atom element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));
      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, $3),
        $4
      };

      $$ = std::make_shared<SetqNode>(t, children);
    }


prog_def:
    "(" SF_PROG list elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      $4.insert($4.begin(), $3);
      $$ = std::make_shared<ProgNode>(t, $4);
    }

cond_def:
    "(" SF_COND element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        $3,
        $4
      };

      $$ = std::make_shared<CondNode>(t, children);
    }
    | "(" SF_COND element element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        $3,
        $4,
        $5
      };

      $$ = std::make_shared<CondNode>(t, children);
    } 


list:
  "(" elements ")" { $$ = std::make_shared<ListNode>($2); }

func_call:
  "(" element elements ")"
  {
    std::vector<std::shared_ptr<ASTNode>> children = {
      $2
    };

    for (auto& e : $3) {
      children.push_back(e);
    }

    $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
  }
  ;

stmt:
  func_def {$$ = $1;}
  | lambda_def {$$ = $1;}
  | quote_def {$$ = $1;}
  | return_def {$$ = $1;}
  | break_def {$$ = $1;}
  | while_def {$$ = $1;}
  | prog_def {$$ = $1;}
  | cond_def {$$ = $1;}
  | setq_def {$$ = $1;}
  | func_call {$$ = $1;}
  ;

sf:
  SF_FUNC { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_LAMBDA { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_QUOTE { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_RETURN { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_BREAK { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_WHILE { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_PROG { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_COND { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  | SF_SETQ { $$ = std::make_shared<Token>(TokenType::KEYWORD, $1, Span({@1.begin.line, @1.begin.column})); }
  ;


atom:
  IDENTIFIER { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  ;

literal:
  INT { $$ = std::make_shared<Token>(TokenType::INT, $1, Span({@1.begin.line, @1.begin.column})); }
  | REAL { $$ = std::make_shared<Token>(TokenType::REAL, $1, Span({@1.begin.line, @1.begin.column})); } 
  | TRUE { $$ = std::make_shared<Token>(TokenType::BOOL, $1, Span({@1.begin.line, @1.begin.column})); } 
  | FALSE { $$ = std::make_shared<Token>(TokenType::BOOL, $1, Span({@1.begin.line, @1.begin.column})); }
  | NULL { $$ = std::make_shared<Token>(TokenType::NUL, $1, Span({@1.begin.line, @1.begin.column})); }

%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << std::endl;
}
