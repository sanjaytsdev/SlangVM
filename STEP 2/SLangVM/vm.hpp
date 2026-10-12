#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cmath>

enum class OpCode {
    PushConst,
    Add,
    Sub,
    Mul,
    Div,
    Neg,
    Halt,
    Enter,
    LoadLocal,
    StoreLocal,
    PrintLine
};

struct Instruction {
    OpCode op;
    double operand = 0;
};

class VirtualMachine {
private:
    std::vector<double> stack;
    std::vector<double> locals;

    double pop() {
        if (stack.empty()) {
            throw std::runtime_error("Operand stack underflow");
        }

        double value = stack.back();
        stack.pop_back();
        return value;
    }

    std::size_t getIndex(double operand) {
        if (!std::isfinite(operand) || operand < 0 || std::floor(operand) != operand || operand > static_cast<double>(locals.size())) {
            throw std::runtime_error("Invalid local slot index");
        }

        return static_cast<std::size_t>(operand);
    }

public:
    double execute(const std::vector<Instruction>& code) {
        stack.clear();
        locals.clear();

        for (std::size_t ip = 0; ip < code.size(); ++ip) {
            const Instruction& ins = code[ip];

            switch (ins.op) {
            case OpCode::PushConst:
                stack.push_back(ins.operand);
                break;

            case OpCode::Enter: {
                double count = ins.operand;

                if (!std::isfinite(count) || count < 0 || std::floor(count) != count || count > 1000000) {
                    throw std::runtime_error("Invalid local variable count");
                }

                locals.assign(static_cast<std::size_t>(count), 0.0);
                break;
            }

            case OpCode::LoadLocal: {
                std::size_t index = getIndex(ins.operand);

                if (index >= locals.size()) {
                    throw std::runtime_error( "Local slot out of range");
                }

                stack.push_back(locals[index]);
                break;
            }

            case OpCode::StoreLocal: {
                std::size_t index = getIndex(ins.operand);

                if (index >= locals.size()) {
                    throw std::runtime_error("Local slot out of range");
                }

                locals[index] = pop();
                break;
            }

            case OpCode::Add: {
                double b = pop();
                double a = pop();
                stack.push_back(a + b);
                break;
            }

            case OpCode::Sub: {
                double b = pop();
                double a = pop();
                stack.push_back(a - b);
                break;
            }

            case OpCode::Mul: {
                double b = pop();
                double a = pop();
                stack.push_back(a * b);
                break;
            }

            case OpCode::Div: {
                double b = pop();
                double a = pop();

                if (b == 0.0) {
                    throw std::runtime_error("Division by zero");
                }

                stack.push_back(a / b);
                break;
            }

            case OpCode::Neg:
                stack.push_back(-pop());
                break;

            case OpCode::PrintLine: {
                if (stack.empty()) {
                    throw std::runtime_error("Operand stack underflow during PRINT_LINE");
                }

                std::cout << stack.back() << '\n';
                stack.pop_back();
                break;
            }

            case OpCode::Halt:
                if (!stack.empty()) {
                    throw std::runtime_error("Expected an empty stack at HALT");
                }

                return 0.0;
            }
        }

        throw std::runtime_error("Program ended without HALT");
    }
};