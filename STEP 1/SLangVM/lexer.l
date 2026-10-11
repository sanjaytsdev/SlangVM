%option noyywrap
%option nounistd
%option never-interactive

%{
#include "parser.tab.h"
#include <cstdlib>
%}

NUMBER_PATTERN ([0-9]+(\.[0-9]*)?|\.[0-9]+)([eE][+-]?[0-9]+)?

%%

"PUSH"      { return PUSH; }
"ADD"       { return ADD; }
"SUB"       { return SUB; }
"MUL"       { return MUL; }
"DIV"       { return DIV; }
"NEG"       { return NEG; }
"HALT"      { return HALT; }

{NUMBER_PATTERN} {
    yylval.number = std::strtod(yytext, nullptr);
    return NUMBER;
}

[ \t\r\n]+  { /* Ignore whitespace */ }

.           { return INVALID; }

%%