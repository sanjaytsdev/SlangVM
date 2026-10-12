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

%token PUSH_CONST ADD SUB MUL DIV NEG HALT ENTER LOAD_LOCAL STORE_LOCAL PRINT_LINE
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
      PUSH_CONST NUMBER
      {
          code.push_back({OpCode::PushConst, $2});
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
    | PRINT_LINE
      {
          code.push_back({OpCode::PrintLine});
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