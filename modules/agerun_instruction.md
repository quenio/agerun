# AgeRun Instruction Module

## Overview

The Instruction module is responsible for parsing and executing instructions in the AgeRun agent system according to the BNF grammar defined in the specification. It uses a recursive descent parser to handle assignment operations and function calls, working in conjunction with the Expression module to evaluate expressions.

## Key Features

- Dedicated context structure for instruction parsing and execution
- Recursive descent parsing implementation for instructions
- Support for assignment operations using memory dot notation
- Support for function calls with optional assignment
- Proper memory management and error handling
- Type-safe operations with clear parsing rules
- Modular design with no dependency on agent module

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
<method-function> ::= 'method' '(' <expression: name> ',' <expression: instructions> ',' <expression: version> ')'
<agent-function> ::= 'agent' '(' <expression> ',' <expression> [',' <expression>] ')'
<destroy-function> ::= 'destroy' '(' <expression> ')'
                     | 'destroy' '(' <expression> ',' <expression> ')'
<if-function> ::= 'if' '(' <comparison-expression> ',' <expression> ',' <expression> ')'

<memory-access> ::= 'memory' {'.' <identifier>}
```

## API Reference

### Instruction Context

```c
instruction_context_t* ar_instruction_create_context(data_t *mut_memory, const data_t *ref_context, const data_t *ref_message);
```

Creates a new instruction context for parsing and executing instructions.

**Parameters:**
- `mut_memory`: The memory to use for the instruction (mutable reference)
- `ref_context`: The context data (borrowed reference)
- `ref_message`: The message being processed (borrowed reference)

**Returns:**
- A newly created instruction context (owned by caller), or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- Does not take ownership of any parameters

```c
void ar_instruction_destroy_context(instruction_context_t *own_ctx);
```

Destroys an instruction context.

**Parameters:**
- `own_ctx`: The instruction context to destroy (ownership transferred to function)

**Ownership:**
- Takes ownership of the context parameter and destroys it
- This only frees the context structure itself, not the memory, context, or message data structures

### Instruction Execution

```c
bool ar_instruction_run(instruction_context_t *mut_ctx, const char *ref_instruction);
```

Parses and executes a single instruction in the context.

**Parameters:**
- `mut_ctx`: The instruction context to use (mutable reference)
- `ref_instruction`: The instruction to execute (borrowed reference)

**Returns:**
- `true` if the instruction was successfully executed
- `false` if parsing failed or execution encountered an error

**Ownership:**
- Does not take ownership of any parameters
- Does not transfer ownership of any objects

### Accessor Functions

```c
data_t* ar_instruction_get_memory(const instruction_context_t *ref_ctx);
```

Gets the memory from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- Mutable reference to the memory (not owned by caller)

```c
const data_t* ar_instruction_get_context(const instruction_context_t *ref_ctx);
```

Gets the context data from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- Borrowed reference to the context data (not owned by caller)

```c
const data_t* ar_instruction_get_message(const instruction_context_t *ref_ctx);
```

Gets the message from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- Borrowed reference to the message (not owned by caller)

### Message Sending

```c
bool ar_instruction_send_message(agent_id_t target_id, data_t *own_message);
```

Sends a message to another agent.

**Parameters:**
- `target_id`: The ID of the agent to send to
- `own_message`: The message to send (ownership transferred)

**Returns:**
- `true` if sending was successful, `false` otherwise

**Ownership:**
- Takes ownership of `own_message`
- If sending fails, the function will destroy the message

## Usage Examples

### Context Creation and Instruction Execution

```c
// Create an instruction context
instruction_context_t *own_ctx = ar_instruction_create_context(mut_memory, ref_context, ref_message);
if (!own_ctx) {
    // Handle error
    return false;
}

// Execute an instruction
bool success = ar_instruction_run(own_ctx, "memory.greeting := \"Hello, World!\"");

// Clean up
ar_instruction_destroy_context(own_ctx);
```

### Assignment Instruction

```c
// Store a string in memory
ar_instruction_run(own_ctx, "memory.greeting := \"Hello, World!\"");

// Store a number in memory
ar_instruction_run(own_ctx, "memory.count := 42");

// Store an expression result in memory
ar_instruction_run(own_ctx, "memory.sum := 2 + 3 * 4");

// Assign a nested value
ar_instruction_run(own_ctx, "memory.user.name := \"John\"");
```

### Function Call Instruction

```c
// Send a message to another agent
ar_instruction_run(own_ctx, "send(target_id, \"Hello\")");

// Parse a string into a structured map
ar_instruction_run(own_ctx, "memory.parsed := parse(\"name={name}\", \"name=John\")");

// Build a string from a template and values
ar_instruction_run(own_ctx, "memory.greeting := build(\"Hello, {name}!\", memory.user)");

// Conditional evaluation
ar_instruction_run(own_ctx, "memory.result := if(memory.count > 5, \"High\", \"Low\")");

// Create a method
ar_instruction_run(own_ctx, "memory.created := method(\"greet\", \"memory.message := \\\"Hello\\\";\", 1)");
```

## Parse Function

The parse function extracts values from strings based on templates:

```c
// Syntax: parse(template, input)
parse("name={name}", "name=John")  // Returns map with name: "John"
parse("user={user}, age={age}", "user=Alice, age=25")  // Returns map with user: "Alice", age: 25
```

**Parameters:**
- `template`: A string containing placeholders in the format `{variable}`
- `input`: The string to parse according to the template

**Returns:**
- A map containing the extracted values
- An empty map if parsing fails (template doesn't match input)

**Implementation Notes:**
- The parse function matches literal parts of the template exactly with the input
- Placeholders in the template (e.g., `{name}`) capture values from the input
- Extracted values are automatically typed: integers, doubles, or strings
- The function handles multiple placeholders in a single template
- If the template doesn't match the input structure, an empty map is returned

## Method Function

The method function allows agents to create and register new methods at runtime:

```c
// Syntax: method(name, instructions, version)
method("greet", "memory.greeting := \"Hello\"", 1)
```

**Parameters:**
- `name`: The name of the method (string)
- `instructions`: The instructions for the method (string)
- `version`: The version number for the method (integer)

**Returns:**
- 1 if the method was successfully created and registered
- 0 if an error occurred

**Implementation Notes:**
- The method function calls `ar_methodology_create_method` directly
- Default values are used for some parameters:
  - `previous_version`: 0 (automatically detected if method exists)
  - `backward_compatible`: true (methods are backward compatible by default)
  - `persist`: false (methods don't persist by default)
- This function facilitates the runtime evolution of agent behaviors

## Implementation Notes

- All instructions follow the BNF grammar from the specification
- Memory access in assignment operations uses dot notation (e.g., `memory.field := value`)
- Function calls can be standalone or part of an assignment
- The parser uses the expression context API to evaluate all expressions
- The parser properly handles whitespace and maintains correct context during parsing
- Function calls delegate to the expression evaluator for argument evaluation
- Memory safety is maintained throughout with proper allocation and cleanup
- Backtracking is used when necessary to determine the correct parsing path
- Expression contexts are reused within function scopes for better efficiency
- Context variables follow a create-use-destroy pattern for each expression evaluation
- The implementation reuses context variables to minimize redundant allocations
- The instruction context structure encapsulates all needed state, removing dependencies on the agent module

## Memory Ownership Model

The instruction module carefully manages memory ownership when working with expressions:

1. When an expression is evaluated using `ar_expression_evaluate()`, the expression context initially owns the result.
2. For assignments (e.g., `memory.x := expression`), the instruction module takes ownership of the result using `ar_expression_take_ownership()` BEFORE destroying the context.
3. When a function call returns a result that needs to be stored, the instruction module assumes ownership of that result.
4. Results that aren't stored (e.g., from functions without assignment) are properly destroyed.

IMPORTANT: The sequence of operations is critical when handling expression results:
1. Create the expression context (`own_context = ar_expression_create_context(...)`)
2. Evaluate the expression to get a result (`own_value = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context))`)
3. Capture the position offset (`*mut_pos += ar_expression_offset(own_context)`)
4. Immediately destroy the context (`ar_expression_destroy_context(own_context)`)
5. Mark the destroyed context as NULL (`own_context = NULL;`)
6. Check if the result is valid
7. Use the result as needed (store in memory, return from function, etc.)
8. Eventually destroy the result when no longer needed
9. Mark the destroyed result as NULL (`own_value = NULL;`)

When reusing the context variable for multiple expressions:
1. Ensure the previous context is fully destroyed before reassigning
2. Follow the same pattern of immediate destruction after each expression
3. Reset the context variable to NULL after each destruction
4. This pattern allows safe reuse while maintaining memory safety

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
- The instruction module no longer depends on the agent module, making it more modular and reusable
- An instruction_context structure is used to hold all required state, similar to the expression_context approach