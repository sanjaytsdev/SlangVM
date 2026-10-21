/*
    win_bison -d --defines=parser.tab.h -o parser.tab.cpp parser.y
    win_flex -o lex.yy.cpp lexer.l
    cl /EHsc /std:c++17 main.cpp parser.tab.cpp lex.yy.cpp /Fe:vm.exe
*/

#include "vm.hpp"

#include <iostream>
#include <vector>
#include <cstdio>
#include <exception>

extern FILE* yyin;
int yyparse();

std::vector<Instruction> code;
std::vector<Function> functions;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./vm <program.vm>\n";
        return 1;
    }

    yyin = std::fopen(argv[1], "r");

    if (yyin == nullptr) {
        std::cerr << "Error: cannot open file: " << argv[1] << '\n';
        return 1;
    }

    int parseResult = yyparse();

    std::fclose(yyin);
    yyin = nullptr;

    if (parseResult != 0) {
        std::cerr << "Program parsing failed.\n";
        return 1;
    }

    try {
        VirtualMachine vm;
        double result = vm.execute(code, functions);
    }
    catch (const std::exception& e) {
        std::cerr << "VM error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}