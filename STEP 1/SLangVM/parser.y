%{
#include "vm.hpp"
#include <vector>
#include <cstdio>

extern std::vector<Instruction> code;

int yylex(void);
void yyerror(const char* message);
%}

%union {
    double number;
}

%token PUSH ADD SUB MUL DIV NEG HALT
%token <number> NUMBER
%token INVALID

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
      PUSH NUMBER
      {
          code.push_back({OpCode::Push, $2});
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
    ;

%%

void yyerror(const char* message) {
    std::fprintf(stderr, "Parser error: %s\n", message);
}