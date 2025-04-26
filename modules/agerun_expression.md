# AgeRun Expression Module

## Overview

The Expression module is responsible for evaluating expressions in the AgeRun agent system according to the BNF grammar defined in the specification. It provides a recursive descent parser that can handle string literals, number literals, memory access paths, arithmetic expressions, and comparison expressions.

## Key Features

- Recursive descent parsing implementation for expressions
- Support for all data types: INTEGER, DOUBLE, STRING, LIST, and MAP
- Memory access using dot notation for message, memory, and context
- Arithmetic operations: addition, subtraction, multiplication, division
- Comparison operations: equals, not equals, less than, greater than, etc.

## BNF Grammar

The expression module implements the following BNF grammar as defined in the specification:

```
<expression> ::= <string-literal>
               | <number-literal>
               | <memory-access>
               | <arithmetic-expression>
               | <comparison-expression>

<string-literal> ::= '"' <characters> '"'

<number-literal> ::= <integer>
                   | <double>

<integer> ::= ['-'] <digit> {<digit>}
<double>  ::= <integer> '.' <digit> {<digit>}

<memory-access> ::= 'message' {'.' <identifier>}
                  | 'memory' {'.' <identifier>}
                  | 'context' {'.' <identifier>}

<arithmetic-expression> ::= <expression> <arithmetic-operator> <expression>
<arithmetic-operator> ::= '+' | '-' | '*' | '/'

<comparison-expression> ::= <expression> <comparison-operator> <expression>
<comparison-operator> ::= '=' | '<>' | '<' | '<=' | '>' | '>='

<identifier> ::= <letter> {<letter> | <digit> | '_'}
<characters> ::= {<any-character-except-quote>}
<digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
<letter> ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'
```

Note that function calls are not part of the expression grammar. Function calls are handled at the instruction level as defined in the specification.

## API Reference

### expression_context_t

```c
typedef struct expression_context_s expression_context_t;
```

An opaque type representing a context for expression evaluation. Contains all information needed during expression parsing and evaluation, including memory map, context map, message being processed, expression string, and current parsing position.

### ar_expression_create_context

```c
expression_context_t* ar_expression_create_context(data_t *memory, data_t *context, data_t *message, const char *expr);
```

Creates a new expression evaluation context.

**Parameters:**
- `memory`: The agent's memory data (can be NULL if not needed)
- `context`: The agent's context data (can be NULL if not needed)
- `message`: The message being processed (can be NULL if not needed)
- `expr`: The expression string to evaluate

**Returns:**
- A newly created expression context, or NULL on failure

### ar_expression_destroy_context

```c
void ar_expression_destroy_context(expression_context_t *ctx);
```

Destroys an expression context.
Note: This only frees the context structure itself, not the memory, context, or message
data structures which are owned by the caller.

**Parameters:**
- `ctx`: The expression context to destroy

### ar_expression_offset

```c
int ar_expression_offset(const expression_context_t *ctx);
```

Gets the current parsing offset in the expression string.

**Parameters:**
- `ctx`: The expression context

**Returns:**
- Current offset in the expression string

### ar_expression_evaluate

```c
data_t* ar_expression_evaluate(expression_context_t *ctx);
```

Evaluates an expression in the agent's context using recursive descent parsing.

**Parameters:**
- `ctx`: Pointer to the expression evaluation context

**Returns:**
- A borrowed reference to the result of the expression evaluation
- This reference is OWNED BY THE EXPRESSION SYSTEM, not the caller
- The caller MUST NOT destroy this reference with ar_data_destroy()
- Returns NULL on syntax error or when the evaluated path doesn't exist

**IMPORTANT: Memory Ownership Clarification**

All results returned by ar_expression_evaluate() are BORROWED REFERENCES, not owned objects. The caller must never attempt to destroy these references using ar_data_destroy(). The expression system manages the lifetime of all expression results internally.

This applies to all expression types:
- Memory access expressions (e.g., `memory.x`)
- Literal expressions (strings, numbers)
- Arithmetic expressions (e.g., `2 + 3`)
- Arithmetic expressions with memory access (e.g., `memory.x + 5`)
- String concatenation (e.g., `"Hello" + " World"`)
- Comparison expressions (e.g., `memory.count > 5`)

The context's offset is updated to point to the position after the evaluated expression on success, or to the position where the syntax error was detected on failure.

## Usage Examples

### Evaluating a String Literal

```c
// All expression results are borrowed references
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "\"Hello, World!\"");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a STRING data with value "Hello, World!"
int position = ar_expression_offset(ctx);

// Use the result
if (result) {
    // Use the result directly
    const char *str = ar_data_get_string(result);
    printf("String value: %s\n", str);
    
    // IMPORTANT: Do NOT destroy the result - it's a borrowed reference
}
ar_expression_destroy_context(ctx);
```

### Evaluating a Number Literal

```c
// Integer evaluation - returns a borrowed reference
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "42");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 42

// Use the result directly
if (result) {
    int value = ar_data_get_integer(result);
    printf("Integer value: %d\n", value);
    
    // IMPORTANT: Do NOT destroy the result - it's a borrowed reference
}
ar_expression_destroy_context(ctx);

// Double evaluation - returns a borrowed reference
ctx = ar_expression_create_context(memory, context, message, "3.14159");
result = ar_expression_evaluate(ctx);
// result will contain a DOUBLE data with value 3.14159

// Use the result directly
if (result) {
    double value = ar_data_get_double(result);
    printf("Double value: %f\n", value);
    
    // IMPORTANT: Do NOT destroy the result - it's a borrowed reference
}
ar_expression_destroy_context(ctx);
```

### Evaluating Memory Access

```c
// Memory access expressions return borrowed references
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "memory.user.name");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a reference to the value stored in memory.user.name, or NULL if path not found

if (result) {
    // Use the result value, but do NOT destroy it
    const char *name = ar_data_get_string(result);
    printf("User name: %s\n", name);
    
    // IMPORTANT: Do NOT call ar_data_destroy(result) - it's a borrowed reference
} else {
    // Handle the case where the path doesn't exist
}
ar_expression_destroy_context(ctx);
```

### Evaluating Arithmetic Expression

```c
// Arithmetic expressions also return borrowed references
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "2 + 3 * 4");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 14

// Use the result
if (result) {
    int value = ar_data_get_integer(result);
    printf("Result: %d\n", value);
    
    // IMPORTANT: Do NOT destroy the result - it's a borrowed reference
}
ar_expression_destroy_context(ctx);
```

### Evaluating Arithmetic with Memory Access

```c
// Arithmetic with memory access returns a borrowed reference
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "memory.count * 2");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a reference to the calculation result

// Use the result
if (result) {
    int value = ar_data_get_integer(result);
    printf("Result: %d\n", value);
    
    // IMPORTANT: Do NOT destroy the result - it's a borrowed reference
}
ar_expression_destroy_context(ctx);
```

### Evaluating Comparison Expression

```c
// Comparison expressions return borrowed references
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "memory.count > 5");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 1 (true) or 0 (false)

// Use the result
if (result) {
    bool is_true = (ar_data_get_integer(result) != 0);
    printf("Comparison result: %s\n", is_true ? "true" : "false");
    
    // IMPORTANT: Do NOT destroy the result - it's a borrowed reference
}
ar_expression_destroy_context(ctx);
```


## Implementation Notes

- The expression context is implemented as an opaque type for improved encapsulation
- Clients interact with the context through a well-defined API (create, destroy, offset, evaluate)
- The module has no dependency on the agent module, maintaining proper layer architecture
- The expression context takes memory and context maps directly instead of requiring an agent
- This design allows for efficient testing and easier reuse in different contexts
- The expression evaluator uses recursive descent parsing to handle nested expressions
- It properly handles precedence of operators (e.g., multiplication before addition)
- Memory access with dot notation is supported for message, memory, and context
- Type conversions are performed automatically where appropriate
- Function calls are detected and treated as syntax errors in expressions
- When a syntax error is encountered, NULL is returned and the context's offset indicates the error location
- This ensures clear distinction between expressions and function calls as specified in the BNF grammar

### Memory Ownership Rules

**CRITICAL UPDATE**: All expression results are now BORROWED REFERENCES.

The expression module follows these critical memory ownership principles:

- **All Expression Results**: For ALL types of expressions (literals, arithmetic, memory access, etc.):
  - The module returns borrowed references that are managed by the expression system
  - These references MUST NEVER be destroyed by the caller using ar_data_destroy()
  - The expression system is responsible for managing the lifetime of all results
  - Attempting to destroy these references will cause segmentation faults

- **Expression Context**: The only resource the caller is responsible for managing is the expression context:
  - The caller creates the context with ar_expression_create_context()
  - The caller must destroy the context with ar_expression_destroy_context() when done
  - Destroying the context does not destroy the result references

This simplified ownership model provides several advantages:
1. Eliminates the risk of double-free errors or memory leaks from incorrect result handling
2. Provides a consistent interface across all expression types
3. Simplifies the caller's responsibility to using results without managing their memory

Failing to follow these rules can lead to crashes from attempting to free borrowed references.