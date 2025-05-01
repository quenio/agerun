# AgeRun Instruction Module

## Overview

The Instruction module is responsible for parsing and executing instructions in the AgeRun agent system according to the BNF grammar defined in the specification. It uses a recursive descent parser to handle assignment operations and function calls, working in conjunction with the Expression module to evaluate expressions.

## Key Features

- Recursive descent parsing implementation for instructions
- Support for assignment operations using memory dot notation
- Support for function calls with optional assignment
- Proper memory management and error handling
- Type-safe operations with clear parsing rules

## BNF Grammar

The instruction module implements the following BNF grammar:

```
<instruction> ::= <assignment>
               | <function-instruction>
               
<assignment> ::= <memory-access> ':=' <expression>

<function-instruction> ::= [<memory-access> ':='] <function-call>

<function-call> ::= <send-function>
                 | <parse-function>
                 | <build-function>
                 | <method-function>
                 | <agent-function>
                 | <destroy-function>
                 | <if-function>

<send-function> ::= 'send' '(' <expression> ',' <expression> ')'
<parse-function> ::= 'parse' '(' <expression> ',' <expression> ')'
<build-function> ::= 'build' '(' <expression> ',' <expression> ')'
<method-function> ::= 'method' '(' <expression> ',' <expression> ',' <expression> ',' <expression> ',' <expression> ',' <expression> ')'
<agent-function> ::= 'agent' '(' <expression> ',' <expression> [',' <expression>] ')'
<destroy-function> ::= 'destroy' '(' <expression> ')'
                     | 'destroy' '(' <expression> ',' <expression> ')'
<if-function> ::= 'if' '(' <comparison-expression> ',' <expression> ',' <expression> ')'

<memory-access> ::= 'memory' {'.' <identifier>}
```

## API Reference

### ar_instruction_run

```c
bool ar_instruction_run(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction);
```

Parses and executes a single instruction in the agent's context.

**Parameters:**
- `mut_agent`: The agent executing the instruction (mutable reference)
- `ref_message`: The message being processed (borrowed reference)
- `ref_instruction`: The instruction string to parse and execute (borrowed reference)

**Returns:**
- `true` if the instruction was successfully executed
- `false` if parsing failed or execution encountered an error

**Ownership:**
- Does not take ownership of any parameters
- Does not transfer ownership of any objects

## Usage Examples

### Assignment Instruction

```c
// Store a string in memory
ar_instruction_run(mut_agent, mut_message, "memory.greeting := \"Hello, World!\"");

// Store a number in memory
ar_instruction_run(mut_agent, mut_message, "memory.count := 42");

// Store an expression result in memory
ar_instruction_run(mut_agent, mut_message, "memory.sum := 2 + 3 * 4");

// Assign a nested value
ar_instruction_run(mut_agent, mut_message, "memory.user.name := \"John\"");
```

### Function Call Instruction

```c
// Send a message to another agent
ar_instruction_run(mut_agent, mut_message, "send(target_id, \"Hello\")");

// Parse a string into a structured map
ar_instruction_run(mut_agent, mut_message, "memory.parsed := parse(\"name={name}\", \"name=John\")");

// Build a string from a template and values
ar_instruction_run(mut_agent, mut_message, "memory.greeting := build(\"Hello, {name}!\", memory.user)");

// Conditional evaluation
ar_instruction_run(mut_agent, mut_message, "memory.result := if(memory.count > 5, \"High\", \"Low\")");
```

## Implementation Notes

- All instructions follow the BNF grammar from the specification
- Memory access in assignment operations uses dot notation (e.g., `memory.field := value`)
- Function calls can be standalone or part of an assignment
- The parser uses the expression context API to evaluate all expressions
- The parser properly handles whitespace and maintains correct context during parsing
- Function calls delegate to the expression evaluator for argument evaluation
- Memory safety is maintained throughout with proper allocation and cleanup
- Backtracking is used when necessary to determine the correct parsing path

## Memory Ownership Model

The instruction module carefully manages memory ownership when working with expressions:

1. When an expression is evaluated using `ar_expression_evaluate()`, the expression context initially owns the result.
2. For assignments (e.g., `memory.x := expression`), the instruction module takes ownership of the result using `ar_expression_take_ownership()` BEFORE destroying the context.
3. When a function call returns a result that needs to be stored, the instruction module assumes ownership of that result.
4. Results that aren't stored (e.g., from functions without assignment) are properly destroyed.

IMPORTANT: The sequence of operations is critical when handling expression results:
1. Create the expression context (`own_ctx = ar_expression_create_context(...)`)
2. Evaluate the expression to get a result (`own_value = ar_expression_evaluate(own_ctx)`)
3. Check if the result is valid
4. If valid and the result needs to be preserved, call `ar_expression_take_ownership(own_ctx, own_value)` 
5. Destroy the expression context (`ar_expression_destroy_context(own_ctx)`)
6. Mark the destroyed context as NULL (`own_ctx = NULL;`)
7. Use the result as needed (store in memory, return from function, etc.)
8. Eventually destroy the result when no longer needed
9. Mark the destroyed result as NULL (`own_value = NULL;`)

This ownership transfer mechanism ensures that:
- No memory leaks occur when an expression result is stored in memory.
- The expression context won't free data that has been stored elsewhere.
- Data is only freed once, preventing use-after-free and double-free errors.
- Results are not accessed after their context has been destroyed, preventing use-after-free bugs.
- All variables explicitly indicate their ownership semantics through prefixes:
  - `own_` prefix for owned values that must be destroyed by the owner
  - `mut_` prefix for mutable references that the function can modify
  - `ref_` prefix for borrowed references that are read-only

## Important Considerations

- Only the `memory` root identifier can be used on the left side of assignments
- Memory access uses exclusively dot notation (e.g., `memory.field.subfield`)
- All assignments use the `:=` operator
- The function call grammar is implemented through the expression evaluator
- After transferring ownership of a pointer, it should be set to NULL immediately
- Variables should always use ownership prefixes to make memory management explicit
- When an owned object is no longer needed, it should be properly destroyed and set to NULL