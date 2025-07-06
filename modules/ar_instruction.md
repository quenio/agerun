# AgeRun Instruction Module

## Overview

The Instruction module is responsible for parsing instructions in the AgeRun agent system according to the BNF grammar defined in the specification. It uses a recursive descent parser to handle assignment operations and function calls, working in conjunction with the Expression module to validate expressions. Execution of parsed instructions is handled by the separate Interpreter module.

## Key Features

- Dedicated context structure for instruction parsing
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
instruction_context_t* ar_instruction__create_context(ar_data_t *mut_memory, const ar_data_t *ref_context, const ar_data_t *ref_message);
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
void ar_instruction__destroy_context(instruction_context_t *own_ctx);
```

Destroys an instruction context.

**Parameters:**
- `own_ctx`: The instruction context to destroy (ownership transferred to function)

**Ownership:**
- Takes ownership of the context parameter and destroys it
- This only frees the context structure itself, not the memory, context, or message data structures

### Instruction Parsing

```c
parsed_instruction_t* ar_instruction__parse(const char *ref_instruction, instruction_context_t *mut_ctx);
```

Parses a single instruction without executing it.

**Parameters:**
- `ref_instruction`: The instruction string to parse (borrowed reference)
- `mut_ctx`: The instruction context for error reporting (mutable reference)

**Returns:**
- Parsed instruction structure on success
- NULL on parse error

**Ownership:**
- Returns an owned value that caller must destroy with ar_instruction__destroy_parsed
- Does not take ownership of any parameters

**Note:** Execution of parsed instructions is handled by the Interpreter module using ar_interpreter__execute_instruction

### Accessor Functions

```c
ar_data_t* ar_instruction__get_memory(const instruction_context_t *ref_ctx);
```

Gets the memory from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- Mutable reference to the memory (not owned by caller)

```c
const ar_data_t* ar_instruction__get_context(const instruction_context_t *ref_ctx);
```

Gets the context data from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- Borrowed reference to the context data (not owned by caller)

```c
const ar_data_t* ar_instruction__get_message(const instruction_context_t *ref_ctx);
```

Gets the message from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- Borrowed reference to the message (not owned by caller)

### Message Sending

```c
bool ar_instruction__send_message(int64_t target_id, ar_data_t *own_message);
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

### Error Reporting

```c
const char* ar_instruction__get_last_error(const instruction_context_t *ref_ctx);
```

Gets the last error message from the instruction context.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- The last error message, or NULL if no error

**Ownership:**
- Does not take ownership of the context parameter
- The returned string is owned by the context and should not be freed

```c
int ar_instruction__get_error_position(const instruction_context_t *ref_ctx);
```

Gets the position in the instruction string where the last error occurred.

**Parameters:**
- `ref_ctx`: The instruction context (borrowed reference)

**Returns:**
- The error position (1-based column), or 0 if no error

**Ownership:**
- Does not take ownership of the context parameter

## Usage Examples

### Context Creation and Instruction Execution

```c
// Create an instruction context
instruction_context_t *own_ctx = ar_instruction__create_context(mut_memory, ref_context, ref_message);
if (!own_ctx) {
    // Handle error
    return false;
}

// Parse an instruction
parsed_instruction_t *own_parsed = ar_instruction__parse("memory.greeting := \"Hello, World!\"", own_ctx);

// Check for errors
if (!own_parsed) {
    const char *error_msg = ar_instruction__get_last_error(own_ctx);
    int error_pos = ar_instruction__get_error_position(own_ctx);
    if (error_msg) {
        fprintf(stderr, "Error at position %d: %s\n", error_pos, error_msg);
    }
} else {
    // Use the parsed instruction (typically passed to interpreter)
    // ar_interpreter__execute_instruction(interpreter, ctx, instruction_string);
    
    // Clean up parsed instruction
    ar_instruction__destroy_parsed(own_parsed);
}

// Clean up context
ar_instruction__destroy_context(own_ctx);
```

### Assignment Instruction Examples

```c
// Parse various assignment instructions
// (Execution would be handled by the interpreter module)

// Store a string in memory
parsed_instruction_t *parsed = ar_instruction__parse("memory.greeting := \"Hello, World!\"", ctx);

// Store a number in memory
parsed = ar_instruction__parse("memory.count := 42", ctx);

// Store an expression result in memory
parsed = ar_instruction__parse("memory.sum := 2 + 3 * 4", ctx);

// Assign a nested value
parsed = ar_instruction__parse("memory.user.name := \"John\"", ctx);
```

### Function Call Instruction Examples

```c
// Parse various function call instructions
// (Execution would be handled by the interpreter module)

// Send a message to another agent
parsed_instruction_t *parsed = ar_instruction__parse("send(target_id, \"Hello\")", ctx);

// Parse a string into a structured map
parsed = ar_instruction__parse("memory.parsed := parse(\"name={name}\", \"name=John\")", ctx);

// Build a string from a template and values
parsed = ar_instruction__parse("memory.greeting := build(\"Hello, {name}!\", memory.user)", ctx);

// Conditional evaluation
parsed = ar_instruction__parse("memory.result := if(memory.count > 5, \"High\", \"Low\")", ctx);

// Create a method
parsed = ar_instruction__parse("memory.created := method(\"greet\", \"memory.message := \\\"Hello\\\";\", 1)", ctx);

// Create an agent
// Note: Map literals {} are not supported in AgeRun expressions
// The context map would need to be created programmatically
parsed = ar_instruction__parse("memory.worker_id := agent(\"echo\", \"1.0.0\", memory.context)", ctx);
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

## Build Function

The build function constructs strings by replacing placeholders in templates with values from a map:

```c
// Syntax: build(template, values)
build("Hello {name}!", {"name": "Alice"})  // Returns "Hello Alice!"
build("Order: {quantity} x {product} at ${price}", {"quantity": 5, "product": "Widget", "price": 19.99})
// Returns "Order: 5 x Widget at $19.99"
```

**Parameters:**
- `template`: A string containing placeholders in the format `{variable}`
- `values`: A map containing key-value pairs to substitute into the template

**Returns:**
- A string with all placeholders replaced by their corresponding values
- Placeholders without matching keys in the values map are left empty
- The original template text for any unmatched braces

**Implementation Notes:**
- The build function searches for `{variable}` patterns in the template
- Values are looked up in the provided map by variable name
- All data types (string, integer, double) are automatically converted to strings
- Missing placeholders result in empty substitution (e.g., `{missing}` becomes ``)
- Unmatched braces (e.g., `{` without closing `}`) are copied literally
- The function dynamically resizes the result buffer as needed
- Empty templates return empty strings
- The function handles nested data structures by accessing map values

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
- The method function calls `ar_methodology__create_method` directly
- Default values are used for some parameters:
  - `previous_version`: 0 (automatically detected if method exists)
  - `backward_compatible`: true (methods are backward compatible by default)
  - `persist`: false (methods don't persist by default)
- This function facilitates the runtime evolution of agent behaviors

## Agent Function

The agent function creates new agent instances at runtime:

```c
// Syntax: agent(method_name, version, context)
memory.context := {"name": "Echo Agent"}
memory.agent_id := agent("echo", "1.0.0", memory.context)  // Returns agent ID
memory.empty := {}
memory.agent_id2 := agent("calculator", "2.0.1", memory.empty)  // With empty context
```

**Parameters:**
- `method_name`: The name of the method for the agent to use (string)
- `version`: The version of the method to use (string, required)
- `context`: Initial context data for the agent (map reference)

**Returns:**
- An integer agent ID if successful (non-zero)
- 0 if the agent creation failed (e.g., method not found)

**Implementation Notes:**
- The agent function calls `ar_agent__create` with the provided parameters
- All parameters are required; version must be a specific version string
- The context parameter must be an expression (e.g., memory reference), not a literal map
- The context is borrowed - the agent will reference it, not take ownership
- Created agents receive a `__wake__` message upon creation
- The returned agent ID can be used to send messages to the new agent
- Returns 0 if the specified method or version doesn't exist

## Destroy Function

The destroy function provides lifecycle management for agents and methods:

```c
// Syntax for destroying an agent:
destroy(agent_id)

// Syntax for unregistering a method:
destroy(method_name, version)
```

### Destroying Agents

```c
// Create an agent
memory.agent_id := agent("echo", "1.0.0", memory.context)

// Later, destroy the agent
memory.success := destroy(memory.agent_id)  // Returns 1 on success, 0 on failure
```

**Parameters:**
- `agent_id`: The ID of the agent to destroy (integer)

**Returns:**
- 1 if the agent was successfully destroyed
- 0 if the agent didn't exist or destruction failed

**Implementation Notes:**
- The function calls `ar_agent__destroy` with the provided agent ID
- The agent receives a `__sleep__` message before being destroyed
- All resources owned by the agent (memory, message queue) are freed
- Attempting to destroy a non-existent agent returns 0

### Unregistering Methods

```c
// Create a method
method("calculator", "memory.result := 0", "1.0.0")

// Later, unregister the method (if no agents are using it)
memory.success := destroy("calculator", "1.0.0")  // Returns 1 on success, 0 on failure
```

**Parameters:**
- `method_name`: The name of the method to unregister (string)
- `version`: The version of the method to unregister (string)

**Returns:**
- 1 if the method was successfully unregistered
- 0 if the method didn't exist, has active agents using it, or unregistration failed

**Implementation Notes:**
- The function calls `ar_methodology__unregister_method` with the provided parameters
- Methods can only be unregistered if no agents are currently using them
- The function checks for active agents before attempting unregistration
- Attempting to unregister a non-existent method returns 0
- After successful unregistration, the methodology is automatically saved to disk
- All resources associated with the method are freed

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

1. When an expression is evaluated using `ar_expression__evaluate()`, the expression context initially owns the result.
2. For assignments (e.g., `memory.x := expression`), the instruction module takes ownership of the result using `ar_expression__take_ownership()` BEFORE destroying the context.
3. When a function call returns a result that needs to be stored, the instruction module assumes ownership of that result.
4. Results that aren't stored (e.g., from functions without assignment) are properly destroyed.

IMPORTANT: The sequence of operations is critical when handling expression results:
1. Create the expression context (`own_context = ar_expression__create_context(...)`)
2. Evaluate the expression to get a result (`own_value = ar_expression__take_ownership(own_context, ar_expression__evaluate(own_context))`)
3. Capture the position offset (`*mut_pos += ar_expression__offset(own_context)`)
4. Immediately destroy the context (`ar_expression__destroy_context(own_context)`)
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