%code requires {
    #include <string>
}

%{
#include "vm.hpp"
#include <vector>
#include <cstdio>
#include <utility>

extern std::vector<Instruction> code;

int yylex(void);
void yyerror(const char* message);
%}

%union {
    double number;
    std::string* string;
}

%token PUSH_CONST PUSH_STRING PUSH_BOOL
%token ADD SUB MUL DIV NEG
%token HALT ENTER LOAD_LOCAL STORE_LOCAL
%token PRINT PRINT_LINE CONCAT POP

%token TRUE FALSE
%token <number> NUMBER
%token <string> STRING
%token INVALID

%destructor { delete $$; } <string>

%start program

%%

program:
    instructions HALT
    {
        code.push_back({OpCode::Halt});
    }
    ;

instructions:
    %empty
    | instructions instruction
    ;

instruction:
      PUSH_CONST NUMBER
      {
          code.push_back({OpCode::PushConst, $2});
      }
    | PUSH_STRING STRING
      {
          code.push_back({
              OpCode::PushString, 0.0, std::move(*$2)
          });
          delete $2;
      }
    | PUSH_BOOL TRUE
      {
          code.push_back({OpCode::PushBool, 0.0, {}, true});
      }
    | PUSH_BOOL FALSE
      {
          code.push_back({OpCode::PushBool, 0.0, {}, false});
      }
    | ENTER NUMBER
      {
          code.push_back({OpCode::Enter, $2});
      }
    | LOAD_LOCAL NUMBER
      {
          code.push_back({OpCode::LoadLocal, $2});
      }
    | STORE_LOCAL NUMBER
      {
          code.push_back({OpCode::StoreLocal, $2});
      }
    | PRINT
      {
          code.push_back({OpCode::Print});
      }
    | PRINT_LINE
      {
          code.push_back({OpCode::PrintLine});
      }
    | CONCAT
      {
          code.push_back({OpCode::Concat});
      }
    | POP
      {
          code.push_back({OpCode::Pop});
      }
    | ADD
      {
          code.push_back({OpCode::Add});
      }
    | SUB
      {
          code.push_back({OpCode::Sub});
      }
    | MUL
      {
          code.push_back({OpCode::Mul});
      }
    | DIV
      {
          code.push_back({OpCode::Div});
      }
    | NEG
      {
          code.push_back({OpCode::Neg});
      }
    | INVALID
      {
          yyerror("invalid token in program");
          YYERROR;
      }
    ;

%%

void yyerror(const char* message) {
    std::fprintf(stderr, "Parser error: %s\n", message);
}