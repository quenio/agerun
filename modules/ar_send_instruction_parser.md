# Send Instruction Parser Module

## Overview

The send instruction parser module is a specialized parser for AgeRun's send function calls. It handles parsing of send instructions with support for complex expressions and nested parentheses.

## Purpose

This module provides:
- Dedicated parsing for send function calls
- Support for optional result assignment
- Handling of nested function calls in arguments
- Detailed error reporting with position information

## Architecture

### Opaque Structure

The module uses an opaque `ar_send_instruction_parser_t` structure that:
- Reports errors through ar_log (deprecated get_error functions return NULL/0)
- Maintains internal parsing state

### Key Functions

- `ar_send_instruction_parser__create(ar_log_t *ref_log)` - Creates a new parser instance
- `ar_send_instruction_parser__destroy()` - Destroys a parser instance
- `ar_send_instruction_parser__parse()` - Parses a send instruction
- `ar_send_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_send_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Usage Example

```c
// Create parser (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_send_instruction_parser_t *parser = ar_send_instruction_parser__create(own_log);

// Parse simple send
ar_instruction_ast_t *ast = ar_send_instruction_parser__parse(
    parser, 
    "send(0, \"Hello\")", 
    NULL
);

// Parse send with assignment
ar_instruction_ast_t *ast2 = ar_send_instruction_parser__parse(
    parser,
    "send(1, \"Test\")",
    "memory.result"
);

// Check for errors
if (!ast) {
    // Parse errors are reported through the ar_log instance
    // The get_error() and get_error_position() functions are deprecated
    printf("Parse error occurred\n");
}

// Clean up
ar_instruction_ast__destroy(ast);
ar_instruction_ast__destroy(ast2);
ar_send_instruction_parser__destroy(parser);
ar_log__destroy(own_log); // Only if log was created
```

## Supported Syntax

The parser handles:
- Basic send: `send(agent_id, message)`
- Complex expressions: `send(memory.agent_id, "Count: " + memory.count)`
- Nested function calls: `send(0, build("Hello {0}", memory.name))`
- Result assignment: `memory.result := send(1, "Query")`

## Error Handling

The parser reports errors through the ar_log instance provided during creation:
- Missing or invalid function name
- Missing parentheses
- Invalid argument count
- Empty or malformed arguments
- Syntax errors in expressions
- The deprecated get_error() and get_error_position() functions always return NULL and 0

## Memory Management

- Parser reports errors through ar_log (deprecated get_error functions return NULL/0)
- Caller owns returned AST nodes
- Arguments are properly cleaned up on parse failure
- Parser can be reused for multiple parse operations

## Dependencies

- `ar_instruction_ast.h` - AST node creation
- `ar_heap.h` - Memory tracking
- `ar_string.h` - String utilities
- Standard C library for string operations

## Testing

Comprehensive test coverage includes:
- Basic send parsing
- Send with assignment
- Expression arguments
- Nested parentheses handling
- Error cases (missing args, invalid syntax)
- Parser reusability
- Memory leak verification