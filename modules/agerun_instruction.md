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
<method-function> ::= 'method' '(' <expression> ',' <expression> ',' <expression> ')'
<agent-function> ::= 'agent' '(' <expression> ',' <expression> [',' <expression>] ')'
<destroy-function> ::= 'destroy' '(' <expression> ')'
                     | 'destroy' '(' <expression> ',' <expression> ')'
<if-function> ::= 'if' '(' <comparison-expression> ',' <expression> ',' <expression> ')'

<memory-access> ::= 'memory' {'.' <identifier>}
```

## API Reference

### ar_instruction_run

```c
bool ar_instruction_run(agent_t *agent, const data_t *message, const char *instruction);
```

Parses and executes a single instruction in the agent's context.

**Parameters:**
- `agent`: The agent executing the instruction
- `message`: The message being processed
- `instruction`: The instruction string to parse and execute

**Returns:**
- `true` if the instruction was successfully executed
- `false` if parsing failed or execution encountered an error

## Usage Examples

### Assignment Instruction

```c
// Store a string in memory
ar_instruction_run(agent, message, "memory.greeting := \"Hello, World!\"");

// Store a number in memory
ar_instruction_run(agent, message, "memory.count := 42");

// Store an expression result in memory
ar_instruction_run(agent, message, "memory.sum := 2 + 3 * 4");

// Assign a nested value
ar_instruction_run(agent, message, "memory.user.name := \"John\"");
```

### Function Call Instruction

```c
// Send a message to another agent
ar_instruction_run(agent, message, "send(target_id, \"Hello\")");

// Parse a string into a structured map
ar_instruction_run(agent, message, "memory.parsed := parse(\"name={name}\", \"name=John\")");

// Build a string from a template and values
ar_instruction_run(agent, message, "memory.greeting := build(\"Hello, {name}!\", memory.user)");

// Conditional evaluation
ar_instruction_run(agent, message, "memory.result := if(memory.count > 5, \"High\", \"Low\")");
```

## Implementation Notes

- All instructions follow the BNF grammar from the specification
- Memory access in assignment operations uses dot notation (e.g., `memory.field := value`)
- Function calls can be standalone or part of an assignment
- The parser uses `ar_expression_evaluate` to evaluate all expressions
- The parser properly handles whitespace and maintains correct context during parsing
- Function calls delegate to the expression evaluator for argument evaluation
- Memory safety is maintained throughout with proper allocation and cleanup
- Backtracking is used when necessary to determine the correct parsing path

## Important Considerations

- Only the `memory` root identifier can be used on the left side of assignments
- Memory access uses exclusively dot notation (e.g., `memory.field.subfield`)
- All assignments use the `:=` operator
- The function call grammar is implemented through the expression evaluator