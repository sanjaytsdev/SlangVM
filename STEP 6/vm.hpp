#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <string>
#include <variant>
#include <utility>
#include <unordered_map>

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

    Label,
    Jump,
    JumpIfFalse,
    JumpIfTrue,

    Call,
    Return,
    ReturnVoid,

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

struct Function {
    std::string name;
    std::size_t parameterCount = 0;
    std::size_t localCount = 0;
    std::size_t entryIp = 0;
};

class VirtualMachine {
private:
    std::vector<Value> stack;
    std::vector<Value> locals;

    struct CallFrame {
        std::size_t returnIp;
        std::size_t localsBase;
        std::size_t localCount;
        std::size_t operandStackBase;
    };

    std::vector<CallFrame> frames;

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
        if (!std::isfinite(operand) || operand < 0 || std::floor(operand) != operand) {
            throw std::runtime_error("Invalid local slot index");
        }

        const std::size_t base = frames.empty() ? 0 : frames.back().localsBase;

        const std::size_t count = frames.empty() ? locals.size() : frames.back().localCount;

        if (operand >= static_cast<double>(count)) {
            throw std::runtime_error("Invalid local slot index");
        }

        return base + static_cast<std::size_t>(operand);
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
    double execute(const std::vector<Instruction>& code, const std::vector<Function>& functions = {}) {
        stack.clear();
        locals.clear();
        frames.clear();

        std::unordered_map<std::string, std::size_t> labels;

        for (std::size_t i = 0; i < code.size(); ++i) {
            if (code[i].op == OpCode::Label) {
                const std::string& name = code[i].text;

                if (!labels.emplace(name, i).second) {
                    throw std::runtime_error("Duplicate label: " + name);
                }
            }
        }

        auto resolveLabel = [&](const std::string& name) -> std::size_t {
            auto it = labels.find(name);

            if (it == labels.end()) {
                throw std::runtime_error("Undefined label: " + name);
            }

            return it->second;
        };

        std::unordered_map<std::string, const Function*> functionMap;

        for (const Function& fn : functions) {
            if (!functionMap.emplace(fn.name, &fn).second) {
                throw std::runtime_error("Duplicate function: " + fn.name);
            }

            if (fn.entryIp >= code.size()) {
                throw std::runtime_error("Invalid entry instruction for function: " + fn.name);
            }

            if (fn.parameterCount > 1000000 || fn.localCount > 1000000 || fn.parameterCount > 1000000 - fn.localCount) {
                throw std::runtime_error("Invalid function frame size: " + fn.name);
            }
        }

        std::size_t ip = 0;

        while (ip < code.size()) {
            const Instruction& ins = code[ip];
            ++ip;

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

                if (!frames.empty()) {
                    throw std::runtime_error("ENTER inside a function is not supported declare locals in FUNCTION metadata");
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

            case OpCode::Label:
                break;

            case OpCode::Jump:
                ip = resolveLabel(ins.text);
                break;

            case OpCode::JumpIfFalse: {
                bool condition = popBool();

                if (!condition) {
                    ip = resolveLabel(ins.text);
                }

                break;
            }

            case OpCode::JumpIfTrue: {
                bool condition = popBool();

                if (condition) {
                    ip = resolveLabel(ins.text);
                }

                break;
            }

            case OpCode::Call: {
                if (!std::isfinite(ins.operand) || ins.operand < 0 || std::floor(ins.operand) != ins.operand || ins.operand > 1000000) {
                    throw std::runtime_error("Invalid argument count");
                }

                const std::size_t argc = static_cast<std::size_t>(ins.operand);

                auto it = functionMap.find(ins.text);

                if (it == functionMap.end()) {
                    throw std::runtime_error("Undefined function: " + ins.text);
                }

                const Function& fn = *it->second;

                if (argc != fn.parameterCount) {
                    throw std::runtime_error("Wrong argument count for function: " + fn.name);
                }

                if (stack.size() < argc) {
                    throw std::runtime_error("Not enough arguments on the operand stack");
                }

                const std::size_t operandBase = stack.size() - argc;
                const std::size_t localsBase = locals.size();

                // Preserve argument order: argument 0 becomes local 0.
                std::vector<Value> args(stack.begin() + operandBase, stack.end());

                // Arguments move from the operand stack into local slots.
                stack.resize(operandBase);

                locals.insert(locals.end(), args.begin(), args.end());

                // Initialize additional local variables to 0.0.
                locals.insert(locals.end(), fn.localCount, Value{ 0.0 });

                frames.push_back(CallFrame{
                    ip,
                    localsBase,
                    fn.parameterCount + fn.localCount,
                    operandBase
                });

                ip = fn.entryIp;
                break;
            }

            case OpCode::Return: {
                if (frames.empty()) {
                    throw std::runtime_error("RETURN outside a function");
                }

                const CallFrame frame = frames.back();

                if (stack.size() != frame.operandStackBase + 1) {
                    throw std::runtime_error("Invalid operand stack at RETURN");
                }

                Value result = popValue();

                frames.pop_back();
                locals.resize(frame.localsBase);

                // Restore the caller's stack and return value.
                stack.resize(frame.operandStackBase);
                stack.push_back(std::move(result));

                ip = frame.returnIp;
                break;
            }

            case OpCode::ReturnVoid: {
                if (frames.empty()) {
                    throw std::runtime_error("RETURN_VOID outside a function");
                }

                const CallFrame frame = frames.back();

                if (stack.size() != frame.operandStackBase) {
                    throw std::runtime_error("Invalid operand stack at RETURN_VOID");
                }

                frames.pop_back();
                locals.resize(frame.localsBase);

                ip = frame.returnIp;
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
                if (!frames.empty()) {
                    throw std::runtime_error("HALT inside a function; use RETURN");
                }

                if (!stack.empty()) {
                    throw std::runtime_error("Expected an empty stack at HALT");
                }

                return 0.0;
            }
        }

        throw std::runtime_error("Program ended without HALT");
    }
};