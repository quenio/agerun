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
- For memory access expressions (`memory`, `context`, `message`):
  - Returns a direct reference to the actual data without copying
  - Returns NULL for non-existent paths
  - IMPORTANT: These references MUST NOT be destroyed by the caller
- For literal expressions (strings, numbers):
  - Returns a new data_t containing the result of the evaluation
  - The caller MUST destroy these objects when no longer needed
- For arithmetic expressions (including those with memory access):
  - Returns a new data_t containing the result of the evaluation
  - The caller MUST destroy these objects, even when they include memory references
  - e.g., For `memory.x + 5`, the result is a new object that must be destroyed
- For comparison expressions:
  - Returns a new data_t containing the result of the evaluation (0 or 1)
  - The caller MUST destroy these objects when no longer needed
- Returns NULL on syntax error (such as encountering a function call in an expression) or when the evaluated path doesn't exist

The context's offset is updated to point to the position after the evaluated expression on success, or to the position where the syntax error was detected on failure.

## Usage Examples

### Evaluating a String Literal

```c
// String literals create new data objects that must be destroyed
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "\"Hello, World!\"");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a STRING data with value "Hello, World!"
int position = ar_expression_offset(ctx);

// Use the result
if (result) {
    // IMPORTANT: String literal results MUST be destroyed
    ar_data_destroy(result);
}
ar_expression_destroy_context(ctx);
```

### Evaluating a Number Literal

```c
// Integer evaluation - creates a new data object
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "42");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 42

// IMPORTANT: Number literal results MUST be destroyed
ar_data_destroy(result);
ar_expression_destroy_context(ctx);

// Double evaluation - creates a new data object
ctx = ar_expression_create_context(memory, context, message, "3.14159");
result = ar_expression_evaluate(ctx);
// result will contain a DOUBLE data with value 3.14159

// IMPORTANT: Number literal results MUST be destroyed
ar_data_destroy(result);
ar_expression_destroy_context(ctx);
```

### Evaluating Memory Access

```c
// Memory access expressions return REFERENCES to existing data
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "memory.user.name");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a direct reference to the value stored in memory.user.name, or NULL if path not found

if (result) {
    // Use the result value, but do NOT destroy it
    const char *name = ar_data_get_string(result);
    printf("User name: %s\n", name);
    
    // IMPORTANT: Do NOT call ar_data_destroy(result) for memory access expressions
} else {
    // Handle the case where the path doesn't exist
}
ar_expression_destroy_context(ctx);
```

### Evaluating Arithmetic Expression

```c
// Arithmetic expressions create new data objects that must be destroyed
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "2 + 3 * 4");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 14

// Use the result
if (result) {
    int value = ar_data_get_integer(result);
    printf("Result: %d\n", value);
    
    // IMPORTANT: Arithmetic expression results MUST be destroyed
    ar_data_destroy(result);
}
ar_expression_destroy_context(ctx);
```

### Evaluating Arithmetic with Memory Access

```c
// Arithmetic with memory access still creates a new data object
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "memory.count * 2");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a new INTEGER data object with the calculation result

// Use the result
if (result) {
    int value = ar_data_get_integer(result);
    printf("Result: %d\n", value);
    
    // IMPORTANT: Even though this uses memory access, the result MUST be destroyed
    ar_data_destroy(result);
}
ar_expression_destroy_context(ctx);
```

### Evaluating Comparison Expression

```c
// Comparison expressions create new integer data objects (0 or 1)
expression_context_t *ctx = ar_expression_create_context(memory, context, message, "memory.count > 5");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 1 (true) or 0 (false)

// Use the result
if (result) {
    bool is_true = (ar_data_get_integer(result) != 0);
    printf("Comparison result: %s\n", is_true ? "true" : "false");
    
    // IMPORTANT: Comparison expression results MUST be destroyed
    ar_data_destroy(result);
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

The module follows these critical memory ownership principles:

- **Memory Access Expressions**: For direct memory access expressions like `memory.x`:
  - The module returns references to existing data, not copies
  - These references MUST NOT be destroyed by the caller
  - Example: `memory.preferences.theme` returns a reference to the theme string value

- **Literal Expressions**: For simple literals like `"Hello"` or `42`:
  - The module creates new data objects containing the value
  - The caller MUST destroy these objects when no longer needed

- **Arithmetic Expressions**: For arithmetic operations like `2 + 3`:
  - The module creates new data objects containing the result
  - The caller MUST destroy these objects when no longer needed

- **Arithmetic with Memory Access**: For expressions like `memory.x + 5`:
  - Even though they involve memory access, these create new data objects
  - The caller MUST destroy these result objects
  - The module handles the proper memory management internally during evaluation
  
- **String Concatenation**: For string operations like `"Hello" + " World"`:
  - The module creates new string objects with the concatenated result
  - The caller MUST destroy these objects when no longer needed

- **Comparison Expressions**: For comparisons like `memory.count > 5`:
  - The module creates new integer data objects with the result (0 or 1)
  - The caller MUST destroy these objects when no longer needed

Failing to follow these rules can lead to memory leaks or crashes from double-free errors.