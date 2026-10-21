%option noyywrap
%option nounistd
%option never-interactive

%{
#include "parser.tab.h"
#include <cstdlib>
#include <stdexcept>
#include <string>

static std::string decodeString(const char* text, int length)
{
    std::string result;

    for (int i = 1; i < length - 1; ++i) {
        if (text[i] == '\\') {
            ++i;

            switch (text[i]) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case 'r':  result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"':  result += '"';  break;
                case '0':  result += '\0'; break;

                default:
                    throw std::runtime_error("Invalid escape sequence");
            }
        } else {
            result += text[i];
        }
    }

    return result;
}
%}

NUMBER_PATTERN ([0-9]+(\.[0-9]*)?|\.[0-9]+)([eE][+-]?[0-9]+)?
VALID_ESCAPE    \\[ntr\\\"0]
STRING_PATTERN \"([^\"\\\n]|{VALID_ESCAPE})*\"
IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_]*

%%

"PUSH_CONST"    { return PUSH_CONST; }
"PUSH_STRING"   { return PUSH_STRING; }
"PUSH_BOOL"     { return PUSH_BOOL; }
"ADD"           { return ADD; }
"SUB"           { return SUB; }
"MUL"           { return MUL; }
"DIV"           { return DIV; }
"NEG"           { return NEG; }
"HALT"          { return HALT; }
"ENTER"         { return ENTER; }
"LOAD_LOCAL"    { return LOAD_LOCAL; }
"STORE_LOCAL"   { return STORE_LOCAL; }
"PRINT_LINE"    { return PRINT_LINE; }
"CONCAT"        { return CONCAT; }
"PRINT"         { return PRINT; }
"POP"           { return POP; }
"true"          { return TRUE; }
"false"         { return FALSE; }
"LE"            { return LE; }
"GE"            { return GE; }
"EQ"            { return EQ; }
"NE"            { return NE; }
"LT"             { return LT; }
"GT"             { return GT; }
"AND"            { return AND; }
"OR"            { return OR; }
"NOT"             { return NOT; }
"LABEL"          { return LABEL; }
"JUMP"           { return JUMP; }
"JUMP_IF_FALSE"  { return JUMP_IF_FALSE; }
"JUMP_IF_TRUE"   { return JUMP_IF_TRUE; }
"FUNCTION"      { return FUNCTION; }
"CALL"          { return CALL; }
"RETURN_VOID"   { return RETURN_VOID; }
"RETURN"        { return RETURN; }
","    { return ','; }

{NUMBER_PATTERN} {
    yylval.number = std::strtod(yytext, nullptr);
    return NUMBER;
}

{STRING_PATTERN} {
    yylval.string = new std::string(
        decodeString(yytext, yyleng)
    );
    return STRING;
}

\"([^\"\\\n]|\\.)*\" {
    return INVALID;
}

\"([^\"\\\n]|\\.)* {
    return INVALID;
}

{IDENTIFIER} {
    yylval.string = new std::string(yytext);
    return IDENTIFIER;
}

[ \t\r\n]+      { /* Ignore whitespace */ }

.               { return INVALID; }

%%