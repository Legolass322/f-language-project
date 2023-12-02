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

%type <std::shared_ptr<FuncCallNode>> plus_def
%type <std::shared_ptr<FuncCallNode>> times_def
%type <std::shared_ptr<FuncCallNode>> divide_def
%type <std::shared_ptr<FuncCallNode>> minus_def

%type <std::shared_ptr<FuncCallNode>> greater_def
%type <std::shared_ptr<FuncCallNode>> equal_def
%type <std::shared_ptr<FuncCallNode>> nonequal_def
%type <std::shared_ptr<FuncCallNode>> less_def
%type <std::shared_ptr<FuncCallNode>> lesseq_def
%type <std::shared_ptr<FuncCallNode>> greatereq_def

%type <std::shared_ptr<FuncCallNode>> isint_def
%type <std::shared_ptr<FuncCallNode>> isreal_def
%type <std::shared_ptr<FuncCallNode>> isbool_def
%type <std::shared_ptr<FuncCallNode>> isnull_def
%type <std::shared_ptr<FuncCallNode>> isatom_def
%type <std::shared_ptr<FuncCallNode>> islist_def

%type <std::shared_ptr<FuncCallNode>> head_def
%type <std::shared_ptr<FuncCallNode>> tail_def
%type <std::shared_ptr<FuncCallNode>> cons_def

%type <std::shared_ptr<FuncCallNode>> not_def
%type <std::shared_ptr<FuncCallNode>> and_def
%type <std::shared_ptr<FuncCallNode>> or_def
%type <std::shared_ptr<FuncCallNode>> xor_def

%type <std::shared_ptr<FuncCallNode>> func_call

%type <std::shared_ptr<FuncCallNode>> eval_def

%type <std::shared_ptr<ASTNode>> program
%type <std::shared_ptr<ASTNode>> element
%type <std::shared_ptr<ASTNode>> stmt 
%type <std::vector<std::shared_ptr<ASTNode>>> elements
%type <std::shared_ptr<ListNode>> list

%type <std::shared_ptr<Token>> atom
%type <std::shared_ptr<Token>> literal

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

quote_def:
    "(" SF_QUOTE elements ")"
    {
      $$ = std::make_shared<ListNode>(true, $3); 
    }
    | SYM_QUOTE "(" elements ")"
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

plus_def:
    "(" PF_PLUS elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);

    }

times_def:
    "(" PF_TIMES elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);
    }

divide_def:
    "(" PF_DIVIDE elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);
    }

minus_def:
    "(" PF_MINUS elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);
    }

head_def:
    "(" PF_HEAD element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

tail_def:
    "(" PF_TAIL element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

cons_def:
    "(" PF_CONS element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
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

not_def:
    "(" PF_NOT element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::KEYWORD, $2, Span({@2.begin.line, @2.begin.column}));
      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
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

greater_def:
    "(" PF_GREATER element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

equal_def:
    "(" PF_EQUAL element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

nonequal_def:
    "(" PF_NONEQUAL element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

less_def:
    "(" PF_LESS element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

lesseq_def:
    "(" PF_LESSEQ element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

greatereq_def:
    "(" PF_GREATEREQ element element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, $2, Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3,
        $4
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

isint_def:
    "(" PF_ISINT element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, "isint", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

isreal_def:
    "(" PF_ISREAL element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER, "isreal", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

isbool_def:
    "(" PF_ISBOOL element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"isbool", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

isnull_def:
    "(" PF_ISNULL element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"isnull", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

isatom_def:
    "(" PF_ISATOM element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"isatom", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

islist_def:
    "(" PF_ISLIST element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"islist", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

eval_def:
    "(" PF_EVAL element ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"eval", Span({@2.begin.line, @2.begin.column}));

      std::vector<std::shared_ptr<ASTNode>> children = {
        std::make_shared<ASTNode>(ASTNodeType::LEAF, t),
        $3
      };

      $$ = std::make_shared<FuncCallNode>(children[0]->head, children);
    }

and_def:
    "(" PF_AND elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"and", Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);
    }

or_def:
    "(" PF_OR elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"or", Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);
    }

xor_def:
    "(" PF_XOR elements ")"
    {
      std::shared_ptr<Token> t = std::make_shared<Token>(TokenType::IDENTIFIER,"xor", Span({@2.begin.line, @2.begin.column}));
      std::shared_ptr<ASTNode> head_node = std::make_shared<ASTNode>(ASTNodeType::LEAF, t);
      $3.insert($3.begin(), head_node);

      $$ = std::make_shared<FuncCallNode>(head_node->head, $3);
    }


list:
  "(" elements ")" { $$ = std::make_shared<ListNode>($2); }

func_call:
  plus_def {$$ = $1;}
  | times_def {$$ = $1;}
  | divide_def {$$ = $1;}
  | minus_def {$$ = $1;}
  | head_def {$$ = $1;}
  | tail_def {$$ = $1;}
  | cons_def {$$ = $1;}
  | not_def {$$ = $1;}
  | greater_def {$$ = $1;}
  | equal_def {$$ = $1;}
  | nonequal_def {$$ = $1;}
  | less_def {$$ = $1;}
  | lesseq_def {$$ = $1;}
  | greatereq_def {$$ = $1;}
  | isint_def {$$ = $1;}
  | isreal_def {$$ = $1;}
  | isbool_def {$$ = $1;}
  | isnull_def {$$ = $1;}
  | isatom_def {$$ = $1;}
  | islist_def {$$ = $1;}
  | eval_def {$$ = $1;}
  | and_def {$$ = $1;}
  | or_def {$$ = $1;}
  | xor_def {$$ = $1;}
  | "(" elements ")"
  {
    std::vector<std::shared_ptr<ASTNode>> children = $2;

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

atom:
  IDENTIFIER { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_PLUS  { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_TIMES { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_DIVIDE { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_MINUS { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_HEAD { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_TAIL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_CONS { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_EQUAL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_NONEQUAL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_LESS { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_LESSEQ { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_GREATER { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_GREATEREQ { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_ISINT { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_ISREAL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_ISBOOL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_ISNULL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_ISATOM { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_ISLIST { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_AND { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_OR { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_XOR { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_NOT { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
  | PF_EVAL { $$ = std::make_shared<Token>(TokenType::IDENTIFIER, $1, Span({@1.begin.line, @1.begin.column})); }
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
