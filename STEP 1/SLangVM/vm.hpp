#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>

enum class OpCode {
    Push,
    Add,
    Sub,
    Mul,
    Div,
    Neg,
    Halt
};

struct Instruction {
    OpCode op;
    double operand = 0;
};

class VirtualMachine {
private:
    std::vector<double> stack;

    double pop() {
        if (stack.empty()) {
            throw std::runtime_error("Operand stack underflow");
        }

        double value = stack.back();
        stack.pop_back();
        return value;
    }

public:
    double execute(const std::vector<Instruction>& code) {
        stack.clear();

        for (std::size_t ip = 0; ip < code.size(); ++ip) {
            const Instruction& ins = code[ip];

            switch (ins.op) {
                case OpCode::Push:
                    stack.push_back(ins.operand);
                    break;

                case OpCode::Add: {
                    double b = pop(), a = pop();
                    stack.push_back(a + b);
                    break;
                }

                case OpCode::Sub: {
                    double b = pop(), a = pop();
                    stack.push_back(a - b);
                    break;
                }

                case OpCode::Mul: {
                    double b = pop(), a = pop();
                    stack.push_back(a * b);
                    break;
                }

                case OpCode::Div: {
                    double b = pop(), a = pop();

                    if (b == 0.0) {
                        throw std::runtime_error("Division by zero");
                    }

                    stack.push_back(a / b);
                    break;
                }

                case OpCode::Neg:
                    stack.push_back(-pop());
                    break;

                case OpCode::Halt:
                    if (stack.size() != 1) {
                        throw std::runtime_error("Expected exactly one result");
                    }
                    return pop();
            }
        }

        throw std::runtime_error("Program ended without HALT");
    }
};