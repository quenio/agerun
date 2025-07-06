# Expression Module

The expression module provides functionality for parsing and evaluating expressions
in the AgeRun agent system. It implements a recursive descent parser that can handle
various expression types including literals, memory access, arithmetic, and comparisons.

## Memory Ownership Model

The expression module carefully manages memory ownership to prevent leaks and use-after-free errors:

1. When `ar_expression__evaluate()` returns a result, the expression context maintains ownership of the result.
2. Direct memory accesses (e.g., `memory.x`) return references that should NOT be destroyed by clients.
3. Other expressions (arithmetic, string concatenation, etc.) create new data objects that the context owns.
4. When `ar_expression__destroy_context()` is called, it automatically frees all results that it owns.
5. To transfer ownership of a result to the caller, use `ar_expression__take_ownership()`.

IMPORTANT: When you need to keep a result after destroying the context, you MUST call 
`ar_expression__take_ownership()` BEFORE calling `ar_expression__destroy_context()`. 
Otherwise, you will have a use-after-free error when attempting to use the result after
the context has been destroyed.

## API Reference

### Types

```c
typedef struct expression_context_s expression_context_t;
```

An opaque type representing the context for expression evaluation. It contains the
expression string, current parsing position, and references to memory, context, and message data.

### Functions

#### ar_expression__create_context

```c
expression_context_t* ar_expression__create_context(ar_data_t *mut_memory, const ar_data_t *ref_context, const ar_data_t *ref_message, const char *ref_expr);
```

Creates a new expression evaluation context.

- **Parameters:**
  - `mut_memory`: The agent's memory data (mutable reference, can be NULL if not needed)
  - `ref_context`: The agent's context data (borrowed reference, can be NULL if not needed)
  - `ref_message`: The message being processed (borrowed reference, can be NULL if not needed)
  - `ref_expr`: The expression string to evaluate (borrowed reference)
- **Returns:** A newly created expression context, or NULL on failure
- **Ownership:** The caller retains ownership of memory, context, and message

#### ar_expression__destroy_context

```c
void ar_expression__destroy_context(expression_context_t *ctx);
```

Destroys an expression context.

- **Parameters:**
  - `ctx`: The expression context to destroy
- **Ownership:** This frees the context structure itself and all expression results owned by the context,
  but not the memory, context, or message data structures which are owned by the caller.

#### ar_expression__offset

```c
int ar_expression__offset(const expression_context_t *ctx);
```

Gets the current parsing offset in the expression string.

- **Parameters:**
  - `ctx`: The expression context
- **Returns:** Current offset in the expression string

#### ar_expression__evaluate

```c
ar_data_t* ar_expression__evaluate(expression_context_t *ctx);
```

Evaluate an expression in the agent's context using recursive descent parsing.

- **Parameters:**
  - `ctx`: Pointer to the expression evaluation context
- **Returns:** Pointer to the evaluated data result, or NULL on failure
- **Ownership:** The context maintains ownership of the returned result. The result will be
  destroyed when the context is destroyed unless ownership is transferred using `ar_expression__take_ownership()`.

#### ar_expression__take_ownership

```c
bool ar_expression__take_ownership(expression_context_t *ctx, ar_data_t *result);
```

Take ownership of a result from the expression context.

- **Parameters:**
  - `ctx`: Pointer to the expression evaluation context
  - `result`: The result to take ownership of
- **Returns:** true if ownership was successfully transferred, false otherwise
- **Ownership:** After a successful call, the caller assumes ownership of the result and becomes
  responsible for eventually destroying it with `ar_data__destroy()`. The context will no longer
  free this result when destroyed.

## Expression Grammar

The expression grammar follows this BNF definition:

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
```

## Usage Examples

```c
// Create an expression context
expression_context_t *ctx = ar_expression__create_context(memory, context, message, "memory.count + 1");

// Evaluate the expression
ar_data_t *result = ar_expression__evaluate(ctx);

// If result is NULL, handle the error and destroy the context
if (!result) {
    ar_expression__destroy_context(ctx);
    return false;
}

// IMPORTANT: Take ownership of the result BEFORE destroying the context if you need to keep it
ar_expression__take_ownership(ctx, result);

// Now that we've taken ownership, we can safely destroy the context
ar_expression__destroy_context(ctx);

// Now you are responsible for destroying the result when done
// Use the result...

// Later, destroy it
ar_data__destroy(result);
```

## Implementation Notes

- The expression evaluator uses recursive descent parsing with operator precedence.
- String concatenation is supported using the '+' operator with at least one string operand.
- Memory access expressions return the actual data references, not copies, for efficiency.
- Arithmetic and comparison expressions create new data objects that the context owns.