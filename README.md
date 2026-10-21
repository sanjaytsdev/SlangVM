# SLang VM

SLang VM is a small stack-based virtual machine for an assembly-like textual bytecode, implemented in C++17 with a Flex lexer and Bison parser. It explores a narrow systems question: how little machinery is required to translate symbolic instructions into deterministic state transitions while keeping value types, control flow, local storage, and function calls explicit? The project is educational in scope, but its subsystems have real boundaries: lexical analysis, parsing, instruction representation, runtime validation, and execution are separate, and every execution rule is defined by the interpreter rather than implied by a higher-level language.

It is not a compiler. Programs are written directly in the VM's instruction language; there is no source-language front end, expression parser, optimizer, or code generator.

## Architecture

The pipeline is:

```text
.vm source -> Flex lexer -> Bison parser -> instruction/function vectors -> VirtualMachine::execute
```

| Subsystem | Responsibility and boundary |
|---|---|
| `lexer.l` | Recognizes opcode keywords, numeric literals, escaped string literals, booleans, identifiers, and commas. Whitespace, including newlines, is insignificant; instructions are a token sequence, not line-oriented records. Invalid tokens become an `INVALID` sentinel. |
| `parser.y` | Uses a flat `instructions instruction` grammar to validate token sequences and emit `Instruction` objects directly. It maintains no AST and performs no name resolution. Declaration and call counts are checked for finiteness, integrality, and range, but runtime arity is checked later. |
| `vm.hpp` instruction model | Defines the opcode enum, a generic `Instruction` record with `double`, `string`, and `bool` operand slots, and `Function` metadata containing name, parameter count, local count, and entry instruction index. |
| `VirtualMachine` | Builds label and function tables, validates declarations, fetches instructions by instruction pointer, and performs all runtime type, stack, and control-flow checks. |
| `main.cpp` | File loading, parser invocation, and the top-level error boundary. Parse or VM failures are reported to stderr with a nonzero exit status. |

The in-memory instruction vector is a decoded representation, not packed bytecode. A `FUNCTION` declaration records the current code length as its entry point; function bodies remain inline in the same instruction stream as top-level code.

## Execution Model

Runtime values are `std::variant<double, bool, std::string>`. There is no integer type: all numbers are doubles. Local slots are dynamically typed and are initialized to `0.0`, but `STORE_LOCAL` accepts any value.

Execution uses three kinds of state:

- An operand stack for expression temporaries and call arguments.
- A single contiguous `locals` vector shared by all frames.
- A stack of `CallFrame` records holding the return instruction pointer, locals base, local count, and operand-stack base.

The instruction pointer starts at index `0` and advances before each dispatch. Unconditional and conditional jumps set it to the absolute index of a `LABEL` instruction, which is itself a no-op. Conditional jumps pop a `bool`; there is no implicit truthiness. `AND` and `OR` are eager boolean operators, not short-circuit control flow. `EQ` and `NE` accept any value pair and treat differing variant alternatives as unequal; relational and arithmetic operators require doubles.

Top-level locals are allocated by `ENTER`. Function locals are declared through `FUNCTION name, parameterCount, localCount`; `ENTER` inside a function is rejected. On `CALL`, the top `argc` operand-stack values become locals `0..argc-1`, additional local slots are zero-filled, a call frame is pushed, and execution jumps to the function entry. Argument order is preserved. `RETURN` requires exactly one value above the frame's operand-stack base, restores the caller's locals and stack, and pushes that value. `RETURN_VOID` requires an empty callee operand stack and produces no value. Recursion and mutual recursion work because each frame carries its own locals base and stack base.

`HALT` is valid only outside a function with an empty operand stack. Running past the final instruction is an error. `execute` returns a constant `0.0`; observable program results are produced through `PRINT` and `PRINT_LINE`.

## Design Philosophy

The design favors explicit semantics over convenience. There are no implicit numeric or boolean conversions, no inferred truthiness, no hidden stack adjustments at function boundaries, and no runtime compilation. Each opcode checks the preconditions it needs, and violations abort execution instead of producing partial results.

Subsystem boundaries are kept narrow: the parser recognizes structure and emits instructions, while the VM owns symbol resolution, call-frame management, and type enforcement. Determinism is a consequence of that split: execution depends only on the instruction stream and function metadata, with stdout as the sole side effect. There is no randomness, concurrency, or external input during execution.

The implementation also accepts deliberate incompleteness. It provides explicit primitives rather than a rich language and avoids abstractions such as heap objects, closures, or an intermediate representation that would obscure the state transitions being studied.

## Key Design Decisions

**Stack-based execution.** Operands are implicit in the current stack state, so instructions are compact and the interpreter's state is easy to inspect. The cost is a larger number of `PUSH`/`POP`-style operations, dynamic stack checks, and no static knowledge of stack depth.

**Tagged values through `std::variant`.** Every value carries its runtime type, allowing one operand stack to hold numbers, booleans, and strings safely. Operations enforce their expected types at use time. The trade-off is per-operation checking and no compile-time type guarantees; local slots are not typed at all.

**Explicit function metadata.** Parameter count, local count, and entry point are declared separately from the body. This gives the runtime a precise call contract and makes frame allocation deterministic. Because bodies are not delimited by a separate structure, control must not fall through into a function body; the supplied example uses `HALT` before function declarations to prevent that.

**Label-based jumps.** Symbolic labels keep programs readable and avoid hard-coded instruction indices. Labels are collected into a map at the start of execution and resolved on every dynamic jump. The trade-off is repeated hash lookups and deferred detection of undefined labels; pre-patching jumps to integer indices would be faster and would permit earlier validation.

**Separate operand stack and locals.** Expression evaluation and variable storage have different access patterns and invariants, so they use distinct structures. Local access is slot-indexed and frame-relative, while operands are consumed by stack discipline. This separation simplifies return cleanup and recursion, at the cost of managing two storage arenas and explicit frame bases.

## Examples

### Arithmetic and Output

```text
PUSH_CONST 6
PUSH_CONST 7
MUL
PRINT_LINE
HALT
```

`MUL` pops two doubles, multiplies them, and pushes the result. `PRINT_LINE` pops and prints it, leaving the operand stack empty for `HALT`. This demonstrates the direct mapping from stack effects to observable output.

### Conditional Branching

```text
PUSH_CONST 4
PUSH_CONST 2
GT
JUMP_IF_FALSE not_greater
PUSH_STRING "greater"
PRINT_LINE
JUMP end
LABEL not_greater
PUSH_STRING "not greater"
PRINT_LINE
LABEL end
HALT
```

`GT` consumes two numbers and pushes a boolean. `JUMP_IF_FALSE` consumes that boolean and transfers control only when it is false. The example shows that conditionals are explicit label transfers, not structured statements, and that each path must leave the stack balanced.

### Function Call

```text
PUSH_CONST 3
PUSH_CONST 4
CALL add, 2
PRINT_LINE
HALT

FUNCTION add, 2, 0
LOAD_LOCAL 0
LOAD_LOCAL 1
ADD
RETURN
```

The call contract requires two arguments and declares no extra locals. `CALL` moves both operands into local slots `0` and `1`, records a frame, and jumps to the first instruction after the declaration. `RETURN` validates the callee's stack shape, restores the caller, and leaves the sum as the sole new operand. The `HALT` is essential: function bodies are inline, so execution would otherwise fall through into `add`.

## Correctness and Failure Semantics

Stack effects are enforced dynamically rather than by a static verifier. Popping an empty stack, consuming a value of the wrong type, or accessing a non-finite, negative, fractional, or out-of-range local slot raises an error. Division by zero is rejected. `RETURN` and `HALT` enforce exact stack shapes, not minimums.

Symbol validation is mixed by necessity. Duplicate labels are rejected before execution, but an undefined label is detected only when a jump to it executes. Duplicate functions, invalid entry indices, and oversized frame declarations are rejected eagerly; an undefined function or an argument-count mismatch is detected when `CALL` executes. The callee must also have enough operands available for its declared argument count.

Return-boundary invariants are strict: `RETURN` requires exactly one result above the active frame's operand-stack base, `RETURN_VOID` requires none, `RETURN` outside a function is invalid, and `HALT` is invalid inside a function or with a nonempty operand stack. A program that reaches the end of the instruction vector without `HALT` fails. Errors propagate as C++ exceptions and are converted by `main` into a diagnostic and exit status `1`; there is no in-language exception or recovery mechanism.

## Limitations and Future Directions

The input language is bytecode, not a source language; there is no compiler front end. There are no aggregate values, references, closures, or heap object graph, and therefore no garbage collector. Strings are ordinary value-semantic `std::string` objects. Numeric behavior is limited to doubles, including exact floating-point equality.

There is no optimizer, JIT, static stack-depth or type verifier, debugger, or source mapping. Undefined symbols in unreachable code are not detected, stack growth and recursion depth are bounded only by host memory, and labels are resolved by map lookup during execution. Function bodies are inline and rely on control flow to avoid falling through; an explicit function-boundary or entry-point convention would make this safer.

Natural extensions include a static validation pass for stack and type invariants, pre-resolved jump targets, bounded call depth and stack limits, a compiler from a small expression language, and additional value types. A garbage collector would become relevant only after introducing heap-allocated reference values. None of these are present in the current implementation.
