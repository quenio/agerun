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

An opaque type representing a context for expression evaluation. Contains all information needed during expression parsing and evaluation, including agent context, message being processed, expression string, and current parsing position.

### ar_expression_create_context

```c
expression_context_t* ar_expression_create_context(agent_t *agent, data_t *message, const char *expr);
```

Creates a new expression evaluation context.

**Parameters:**
- `agent`: The agent context (can be NULL for standalone evaluation)
- `message`: The message being processed (can be NULL if not needed)
- `expr`: The expression string to evaluate

**Returns:**
- A newly created expression context, or NULL on failure

### ar_expression_destroy_context

```c
void ar_expression_destroy_context(expression_context_t *ctx);
```

Destroys an expression context and frees all associated resources.

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
- For other expressions (literals, arithmetic, comparison):
  - Returns a new data_t containing the result of the evaluation
- Returns NULL on syntax error (such as encountering a function call in an expression) or when the evaluated path doesn't exist

The context's offset is updated to point to the position after the evaluated expression on success, or to the position where the syntax error was detected on failure.

## Usage Examples

### Evaluating a String Literal

```c
expression_context_t *ctx = ar_expression_create_context(agent, message, "\"Hello, World!\"");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a STRING data with value "Hello, World!"
int position = ar_expression_offset(ctx);
ar_expression_destroy_context(ctx);
```

### Evaluating a Number Literal

```c
// Integer evaluation
expression_context_t *ctx = ar_expression_create_context(agent, message, "42");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 42
ar_expression_destroy_context(ctx);

// Double evaluation
ctx = ar_expression_create_context(agent, message, "3.14159");
result = ar_expression_evaluate(ctx);
// result will contain a DOUBLE data with value 3.14159
ar_expression_destroy_context(ctx);
```

### Evaluating Memory Access

```c
expression_context_t *ctx = ar_expression_create_context(agent, message, "memory.user.name");
data_t *result = ar_expression_evaluate(ctx);
// result will contain a direct reference to the value stored in memory.user.name, or NULL if path not found
if (result) {
    // Use the result value
} else {
    // Handle the case where the path doesn't exist
}
ar_expression_destroy_context(ctx);
```

### Evaluating Arithmetic Expression

```c
expression_context_t *ctx = ar_expression_create_context(agent, message, "2 + 3 * 4");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 14
ar_expression_destroy_context(ctx);
```

### Evaluating Comparison Expression

```c
expression_context_t *ctx = ar_expression_create_context(agent, message, "memory.count > 5");
data_t *result = ar_expression_evaluate(ctx);
// result will contain an INTEGER data with value 1 (true) or 0 (false)
ar_expression_destroy_context(ctx);
```


## Implementation Notes

- The expression context is implemented as an opaque type for improved encapsulation
- Clients interact with the context through a well-defined API (create, destroy, offset, evaluate)
- This design allows for future changes to the internal structure without affecting client code
- The expression evaluator uses recursive descent parsing to handle nested expressions
- It properly handles precedence of operators (e.g., multiplication before addition)
- Memory access with dot notation is supported for message, memory, and context
- Type conversions are performed automatically where appropriate
- Function calls are detected and treated as syntax errors in expressions
- When a syntax error is encountered, NULL is returned and the context's offset indicates the error location
- This ensures clear distinction between expressions and function calls as specified in the BNF grammar