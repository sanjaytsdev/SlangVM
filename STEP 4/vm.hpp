#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <string>
#include <variant>
#include <utility>

using Value = std::variant<double, bool, std::string>;

enum class OpCode {
    PushConst,
    PushString,
    PushBool,

    Add,
    Sub,
    Mul,
    Div,
    Neg,
    Concat,

    Lt,
    Gt,
    Le,
    Ge,
    Eq,
    Ne,

    And,
    Or,
    Not,

    Halt,
    Enter,
    LoadLocal,
    StoreLocal,

    Print,
    PrintLine,
    Pop
};

struct Instruction {
    OpCode op;
    double operand = 0.0;
    std::string text;
    bool boolean = false;
};

class VirtualMachine {
private:
    std::vector<Value> stack;
    std::vector<Value> locals;

    Value popValue() {
        if (stack.empty()) {
            throw std::runtime_error("Operand stack underflow");
        }

        Value value = std::move(stack.back());
        stack.pop_back();
        return value;
    }

    double popNumber() {
        Value value = popValue();

        if (auto number = std::get_if<double>(&value)) {
            return *number;
        }

        throw std::runtime_error("Type error: expected a number");
    }

    bool popBool() {
        Value value = popValue();

        if (auto boolean = std::get_if<bool>(&value)) {
            return *boolean;
        }

        throw std::runtime_error("Type error: expected a Boolean");
    }

    std::string popString() {
        Value value = popValue();

        if (auto str = std::get_if<std::string>(&value)) {
            return std::move(*str);
        }

        throw std::runtime_error("Type error: expected a string");
    }

    std::size_t getIndex(double operand) const {
        if (!std::isfinite(operand) || operand < 0 || std::floor(operand) != operand || operand >= static_cast<double>(locals.size())) {
            throw std::runtime_error("Invalid local slot index");
        }

        return static_cast<std::size_t>(operand);
    }

    static void printValue(const Value& value) {
        if (auto number = std::get_if<double>(&value)) {
            std::cout << *number;
        }
        else if (auto boolean = std::get_if<bool>(&value)) {
            std::cout << (*boolean ? "true" : "false");
        }
        else if (auto str = std::get_if<std::string>(&value)) {
            std::cout << *str;
        }
    }

public:
    double execute(const std::vector<Instruction>& code) {
        stack.clear();
        locals.clear();

        for (std::size_t ip = 0; ip < code.size(); ++ip) {
            const Instruction& ins = code[ip];

            switch (ins.op) {
            case OpCode::PushConst:
                stack.emplace_back(ins.operand);
                break;

            case OpCode::PushString:
                stack.emplace_back(ins.text);
                break;

            case OpCode::PushBool:
                stack.emplace_back(ins.boolean);
                break;

            case OpCode::Enter: {
                double count = ins.operand;

                if (!std::isfinite(count) || count < 0 || std::floor(count) != count || count > 1000000) {
                    throw std::runtime_error("Invalid local variable count");
                }

                locals.assign(static_cast<std::size_t>(count), Value{ 0.0 });
                break;
            }

            case OpCode::LoadLocal: {
                std::size_t index = getIndex(ins.operand);
                stack.push_back(locals[index]);
                break;
            }

            case OpCode::StoreLocal: {
                std::size_t index = getIndex(ins.operand);
                locals[index] = popValue();
                break;
            }

            case OpCode::Add: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a + b);
                break;
            }

            case OpCode::Sub: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a - b);
                break;
            }

            case OpCode::Mul: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a * b);
                break;
            }

            case OpCode::Div: {
                double b = popNumber();
                double a = popNumber();

                if (b == 0.0) {
                    throw std::runtime_error("Division by zero");
                }

                stack.emplace_back(a / b);
                break;
            }

            case OpCode::Neg: {
                double value = popNumber();
                stack.emplace_back(-value);
                break;
            }

            case OpCode::Lt: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a < b);
                break;
            }

            case OpCode::Gt: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a > b);
                break;
            }

            case OpCode::Le: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a <= b);
                break;
            }

            case OpCode::Ge: {
                double b = popNumber();
                double a = popNumber();
                stack.emplace_back(a >= b);
                break;
            }

            case OpCode::Eq: {
                Value b = popValue();
                Value a = popValue();

                if (a.index() != b.index()) {
                    stack.emplace_back(false);
                }
                else if (auto x = std::get_if<double>(&a)) {
                    stack.emplace_back(*x == std::get<double>(b));
                }
                else if (auto x = std::get_if<bool>(&a)) {
                    stack.emplace_back(*x == std::get<bool>(b));
                }
                else {
                    stack.emplace_back(std::get<std::string>(a) == std::get<std::string>(b));
                }

                break;
            }

            case OpCode::Ne: {
                Value b = popValue();
                Value a = popValue();

                if (a.index() != b.index()) {
                    stack.emplace_back(true);
                }
                else if (auto x = std::get_if<double>(&a)) {
                    stack.emplace_back(*x != std::get<double>(b));
                }
                else if (auto x = std::get_if<bool>(&a)) {
                    stack.emplace_back(*x != std::get<bool>(b));
                }
                else {
                    stack.emplace_back(std::get<std::string>(a) != std::get<std::string>(b));
                }

                break;
            }

            case OpCode::And: {
                bool b = popBool();
                bool a = popBool();
                stack.emplace_back(a && b);
                break;
            }

            case OpCode::Or: {
                bool b = popBool();
                bool a = popBool();
                stack.emplace_back(a || b);
                break;
            }

            case OpCode::Not: {
                bool value = popBool();
                stack.emplace_back(!value);
                break;
            }

            case OpCode::Concat: {
                std::string right = popString();
                std::string left = popString();

                left += right;
                stack.emplace_back(std::move(left));
                break;
            }

            case OpCode::Print: {
                Value value = popValue();
                printValue(value);
                break;
            }

            case OpCode::PrintLine: {
                Value value = popValue();
                printValue(value);
                std::cout << '\n';
                break;
            }

            case OpCode::Pop:
                popValue();
                break;

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