%code requires {
    #include <string>
    #include <cmath>
}

%{
#include "vm.hpp"
#include <vector>
#include <cstdio>
#include <utility>

extern std::vector<Instruction> code;
extern std::vector<Function> functions;

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
%token LT GT LE GE EQ NE
%token AND OR NOT
%token FUNCTION CALL RETURN RETURN_VOID

%token TRUE FALSE
%token <number> NUMBER
%token <string> STRING
%token INVALID
%token LABEL JUMP JUMP_IF_FALSE JUMP_IF_TRUE
%token <string> IDENTIFIER

%destructor { delete $$; } <string>

%start program

%%

program:
    instructions
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
    | LT  
      { 
          code.push_back({OpCode::Lt}); 
      }
    | GT  
      { 
          code.push_back({OpCode::Gt}); 
      }
    | LE  
      { 
          code.push_back({OpCode::Le}); 
      }
    | GE  
      { 
          code.push_back({OpCode::Ge}); 
      }
    | EQ  
      { 
          code.push_back({OpCode::Eq}); 
      }
    | NE  
      { 
          code.push_back({OpCode::Ne}); 
      }
    | AND 
      { 
          code.push_back({OpCode::And}); 
      }
    | OR  
      { 
          code.push_back({OpCode::Or}); 
      }
    | NOT 
      { 
          code.push_back({OpCode::Not}); 
      }
    | LABEL IDENTIFIER
      {
          code.push_back({
              OpCode::Label, 0.0, std::move(*$2)
          });
          delete $2;
      }
    | JUMP IDENTIFIER
      {
          code.push_back({
              OpCode::Jump, 0.0, std::move(*$2)
          });
          delete $2;
      }
    | JUMP_IF_FALSE IDENTIFIER
      {
          code.push_back({
              OpCode::JumpIfFalse, 0.0, std::move(*$2)
          });
          delete $2;
      }
    | JUMP_IF_TRUE IDENTIFIER
      {
          code.push_back({
              OpCode::JumpIfTrue, 0.0, std::move(*$2)
          });
          delete $2;
      }
    | FUNCTION IDENTIFIER ',' NUMBER ',' NUMBER
      {
          if (!std::isfinite($4) || !std::isfinite($6) || $4 < 0 || std::floor($4) != $4 || $4 > 1000000 || $6 < 0 || std::floor($6) != $6 || $6 > 1000000 || $4 + $6 > 1000000) {
                delete $2;
                yyerror("invalid function parameter or local count");
                YYERROR;
          }

          std::string name = std::move(*$2);
          delete $2;

          functions.push_back(Function{
              name,
              static_cast<std::size_t>($4),
              static_cast<std::size_t>($6),
              code.size()
          });
      }
    | CALL IDENTIFIER ',' NUMBER
      {
          if (!std::isfinite($4) || $4 < 0 || std::floor($4) != $4 || $4 > 1000000) {
              delete $2;
              yyerror("invalid argument count");
              YYERROR;
          }

          code.push_back({
              OpCode::Call,
              $4,
              std::move(*$2)
          });

          delete $2;
      }
    | RETURN
      {
          code.push_back({OpCode::Return});
      }
    | RETURN_VOID
      {
          code.push_back({OpCode::ReturnVoid});
      }
    | INVALID
      {
          yyerror("invalid token in program");
          YYERROR;
      }
    | HALT
      {
          code.push_back({OpCode::Halt});
      }
    ;

%%

void yyerror(const char* message) {
    std::fprintf(stderr, "Parser error: %s\n", message);
}