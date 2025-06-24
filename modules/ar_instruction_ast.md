# AgeRun Instruction AST Module

## Overview

The Instruction AST module provides Abstract Syntax Tree (AST) representations for AgeRun instructions. It defines node types for all instruction types in the language and provides functions to create, access, and destroy these nodes. This module is designed to work with the instruction parser to create a clean separation between parsing and execution phases.

## Key Features

- AST node types for all AgeRun instruction types
- Type-safe node creation with proper memory management
- Accessor functions with clear ownership semantics
- Support for both assignment and function call instructions
- Returns lists for function arguments (ownership transferred to caller)
- Comprehensive null safety and edge case handling

## Instruction Types

The module supports the following instruction types:

- **AR_INST__ASSIGNMENT**: Assignment instructions (e.g., `memory.x := 42`)
- **AR_INST__SEND**: Send function calls (e.g., `send(0, "Hello")`)
- **AR_INST__IF**: Conditional function calls (e.g., `if(x > 5, "High", "Low")`)
- **AR_INST__METHOD**: Method creation calls (e.g., `method("greet", "...", "1.0.0")`)
- **AR_INST__AGENT**: Agent creation calls (e.g., `agent("echo", "1.0.0", context)`)
- **AR_INST__DESTROY_AGENT**: Destroy agent calls (e.g., `destroy(agent_id)`)
- **AR_INST__DESTROY_METHOD**: Destroy method calls (e.g., `destroy("method_name", "1.0.0")`)
- **AR_INST__PARSE**: Parse function calls (e.g., `parse("{name}", "name=John")`)
- **AR_INST__BUILD**: Build function calls (e.g., `build("Hello {name}", map)`)

## API Reference

### Type Definitions

```c
typedef enum {
    AR_INST__ASSIGNMENT,
    AR_INST__SEND,
    AR_INST__IF,
    AR_INST__METHOD,
    AR_INST__AGENT,
    AR_INST__DESTROY_AGENT,
    AR_INST__DESTROY_METHOD,
    AR_INST__PARSE,
    AR_INST__BUILD
} ar_instruction_ast_type_t;

typedef struct ar_instruction_ast_s ar_instruction_ast_t;
```

### Node Creation

#### Assignment Instructions

```c
ar_instruction_ast_t* ar__instruction_ast__create_assignment(
    const char *ref_memory_path,
    const char *ref_expression
);
```

Creates an AST node for an assignment instruction.

**Parameters:**
- `ref_memory_path`: The memory path (e.g., "memory.x.y") (borrowed reference)
- `ref_expression`: The expression to assign (borrowed reference)

**Returns:**
- Newly created AST node (owned by caller), or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- Makes copies of all string parameters

#### Function Call Instructions

```c
ar_instruction_ast_t* ar__instruction_ast__create_function_call(
    ar_instruction_ast_type_t type,
    const char *ref_function_name,
    const char **ref_args,
    size_t arg_count,
    const char *ref_result_path
);
```

Creates an AST node for a function call instruction.

**Parameters:**
- `type`: The function type (must be one of the function instruction types)
- `ref_function_name`: The function name (borrowed reference)
- `ref_args`: Array of argument expressions (borrowed references, can be NULL)
- `arg_count`: Number of arguments
- `ref_result_path`: Optional result assignment path (borrowed reference, can be NULL)

**Returns:**
- Newly created AST node (owned by caller), or NULL on failure

**Ownership:**
- Returns an owned value that caller must destroy
- Makes copies of all string parameters

### Node Destruction

```c
void ar__instruction_ast__destroy(ar_instruction_ast_t *own_node);
```

Destroys an AST node and all its components.

**Parameters:**
- `own_node`: The AST node to destroy (ownership transferred to function)

**Ownership:**
- Takes ownership of the node and destroys it
- Safe to call with NULL

### Accessor Functions

#### General Node Information

```c
ar_instruction_ast_type_t ar__instruction_ast__get_type(const ar_instruction_ast_t *ref_node);
```

Gets the type of an AST node.

**Returns:**
- The type of the node, or AR_INST__ASSIGNMENT if node is NULL

#### Assignment Node Accessors

```c
const char* ar__instruction_ast__get_assignment_path(const ar_instruction_ast_t *ref_node);
const char* ar__instruction_ast__get_assignment_expression(const ar_instruction_ast_t *ref_node);
```

Get information from assignment nodes.

**Returns:**
- Borrowed references to the stored strings, or NULL if not an assignment node

#### Function Call Node Accessors

```c
const char* ar__instruction_ast__get_function_name(const ar_instruction_ast_t *ref_node);
list_t* ar__instruction_ast__get_function_args(const ar_instruction_ast_t *ref_node);
const char* ar__instruction_ast__get_function_result_path(const ar_instruction_ast_t *ref_node);
bool ar__instruction_ast__has_result_assignment(const ar_instruction_ast_t *ref_node);
```

Get information from function call nodes.

**Ownership:**
- `ar__instruction_ast__get_function_args` returns an owned list that caller must destroy
- The strings in the returned list are borrowed references and should not be freed
- Other functions return borrowed references

## Usage Examples

### Creating an Assignment Node

```c
// Create an assignment: memory.count := 42
ar_instruction_ast_t *own_node = ar__instruction_ast__create_assignment("memory.count", "42");
if (own_node) {
    // Use the node...
    assert(ar__instruction_ast__get_type(own_node) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_node), "memory.count") == 0);
    
    // Clean up
    ar__instruction_ast__destroy(own_node);
}
```

### Creating a Function Call Node

```c
// Create a send call: send(0, "Hello")
const char *args[] = {"0", "\"Hello\""};
ar_instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
    AR_INST__SEND, "send", args, 2, NULL
);

if (own_node) {
    // Access function arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_node);
    if (own_args) {
        void **own_items = ar__list__items(own_args);
        // Use the arguments...
        AR__HEAP__FREE(own_items);
        ar__list__destroy(own_args);
    }
    
    // Clean up
    ar__instruction_ast__destroy(own_node);
}
```

### Creating a Function Call with Result Assignment

```c
// Create: memory.result := if(x > 5, "High", "Low")
const char *args[] = {"x > 5", "\"High\"", "\"Low\""};
ar_instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
    AR_INST__IF, "if", args, 3, "memory.result"
);

if (own_node) {
    assert(ar__instruction_ast__has_result_assignment(own_node) == true);
    assert(strcmp(ar__instruction_ast__get_function_result_path(own_node), "memory.result") == 0);
    
    ar__instruction_ast__destroy(own_node);
}
```

## Memory Management

The module follows the AgeRun Memory Management Model (MMM):

1. **Node Creation**: All creation functions return owned nodes that the caller must destroy
2. **String Copying**: All string parameters are copied during node creation
3. **Argument Lists**: The `get_function_args` function returns an owned list
4. **Borrowed References**: All other accessor functions return borrowed references
5. **Safe Destruction**: The destroy function handles NULL nodes safely
6. **Initialization**: All fields are properly initialized to prevent uninitialized access

## Error Handling

- Creation functions return NULL on allocation failure
- Accessor functions return appropriate defaults for NULL nodes
- Type checking ensures accessors return NULL for wrong node types
- All allocation failures are handled gracefully with proper cleanup

## Testing

The module includes comprehensive tests covering:
- All instruction types
- Assignment and function call variations
- Function calls with and without result assignment
- NULL handling and edge cases
- Memory leak verification
- Argument list ownership semantics
