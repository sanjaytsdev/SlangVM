%option noyywrap
%option nounistd
%option never-interactive

%{
#include "parser.tab.h"
#include <cstdlib>
%}

NUMBER_PATTERN ([0-9]+(\.[0-9]*)?|\.[0-9]+)([eE][+-]?[0-9]+)?

%%

"PUSH_CONST"      { return PUSH_CONST; }
"ADD"       { return ADD; }
"SUB"       { return SUB; }
"MUL"       { return MUL; }
"DIV"       { return DIV; }
"NEG"       { return NEG; }
"HALT"      { return HALT; }
"ENTER"         { return ENTER; }
"LOAD_LOCAL"    { return LOAD_LOCAL; }
"STORE_LOCAL"   { return STORE_LOCAL; }
"PRINT_LINE"    { return PRINT_LINE; }

{NUMBER_PATTERN} {
    yylval.number = std::strtod(yytext, nullptr);
    return NUMBER;
}

[ \t\r\n]+  { /* Ignore whitespace */ }

.           { return INVALID; }

%%