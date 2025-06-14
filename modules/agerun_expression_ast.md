# Expression AST Module

## Overview

The expression AST module (`agerun_expression_ast`) provides Abstract Syntax Tree (AST) structures for representing parsed expressions in the AgeRun language. This module defines the data structures and operations for building, manipulating, and destroying expression ASTs without any dependency on the expression evaluation module.

## Purpose

The expression AST module serves as the foundation for separating expression parsing from evaluation. It provides:

1. **AST Structure Definition**: Comprehensive node types for all AgeRun expression types
2. **Type-Safe Construction**: Functions to create AST nodes with proper ownership semantics
3. **Safe Destruction**: Recursive cleanup of AST trees with no memory leaks
4. **Clean Separation**: No dependencies on expression parsing or evaluation logic

## Design Principles

### Opaque Type Pattern

The module uses opaque types to hide implementation details:

```c
typedef struct expression_ast_s expression_ast_t;
```

This ensures that clients cannot directly access or modify the internal structure of AST nodes.

### Node Type Enumeration

The module defines all possible expression types:

```c
typedef enum {
    EXPR_AST_LITERAL_INT,      /* Integer literal (e.g., 42, -10) */
    EXPR_AST_LITERAL_DOUBLE,   /* Double literal (e.g., 3.14, -2.5) */
    EXPR_AST_LITERAL_STRING,   /* String literal (e.g., "hello") */
    EXPR_AST_MEMORY_ACCESS,    /* Memory/message/context access */
    EXPR_AST_BINARY_OP         /* Binary operation (arithmetic or comparison) */
} expression_ast_type_t;
```

### Binary Operator Types

All binary operators are defined in a single enumeration:

```c
typedef enum {
    /* Arithmetic operators */
    OP_ADD,        /* + */
    OP_SUBTRACT,   /* - */
    OP_MULTIPLY,   /* * */
    OP_DIVIDE,     /* / */
    
    /* Comparison operators */
    OP_EQUAL,      /* = */
    OP_NOT_EQUAL,  /* <> */
    OP_LESS,       /* < */
    OP_LESS_EQ,    /* <= */
    OP_GREATER,    /* > */
    OP_GREATER_EQ  /* >= */
} binary_operator_t;
```

## API Functions

### Node Creation

Each node type has its own creation function:

- `ar__expression_ast__create_literal_int(int value)` - Creates integer literal nodes
- `ar__expression_ast__create_literal_double(double value)` - Creates double literal nodes
- `ar__expression_ast__create_literal_string(const char *ref_value)` - Creates string literal nodes
- `ar__expression_ast__create_memory_access(base, path, count)` - Creates memory access nodes
- `ar__expression_ast__create_binary_op(op, left, right)` - Creates binary operation nodes

### Node Inspection

Type-safe accessor functions for retrieving node data:

- `ar__expression_ast__get_type()` - Returns the node type
- `ar__expression_ast__get_int_value()` - Gets integer value from literal nodes
- `ar__expression_ast__get_double_value()` - Gets double value from literal nodes
- `ar__expression_ast__get_string_value()` - Gets string value from literal nodes
- `ar__expression_ast__get_memory_base()` - Gets base accessor from memory nodes
- `ar__expression_ast__get_memory_path()` - Gets path components from memory nodes
- `ar__expression_ast__get_operator()` - Gets operator from binary nodes
- `ar__expression_ast__get_left()` - Gets left operand from binary nodes
- `ar__expression_ast__get_right()` - Gets right operand from binary nodes

### Memory Management

- `ar__expression_ast__destroy()` - Recursively destroys an AST tree

## Memory Ownership

The module follows strict ownership semantics:

1. **Creation Functions**: Return owned nodes that the caller must destroy
2. **Binary Operations**: Take ownership of both operand nodes
3. **String Values**: The module makes copies of all string parameters
4. **Accessor Functions**: Return borrowed references, except for `get_memory_path()` which transfers array ownership
5. **Destruction**: Recursively frees all child nodes and owned memory

## Usage Example

```c
// Create an AST for: memory.x + 5
const char *path[] = {"x"};
expression_ast_t *own_mem = ar__expression_ast__create_memory_access("memory", path, 1);
expression_ast_t *own_five = ar__expression_ast__create_literal_int(5);
expression_ast_t *own_add = ar__expression_ast__create_binary_op(OP_ADD, own_mem, own_five);

// Inspect the AST
assert(ar__expression_ast__get_type(own_add) == EXPR_AST_BINARY_OP);
assert(ar__expression_ast__get_operator(own_add) == OP_ADD);

// Clean up
ar__expression_ast__destroy(own_add);
```

## Dependencies

- **agerun_heap** - For memory allocation and tracking
- **agerun_list** - For storing memory access path components
- **agerun_assert** - For ownership assertions in debug builds

## Testing

The module includes comprehensive tests (`agerun_expression_ast_tests.c`) that verify:

- Creation of all node types
- Accessor function behavior
- Memory ownership and cleanup
- NULL safety
- Complex nested expressions

All tests follow the Given/When/Then pattern and verify zero memory leaks.