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

The module uses an opaque `ar_send_instruction_parser_t` structure that maintains:
- Error message from last parse attempt
- Character position where error occurred

### Key Functions

- `ar_send_instruction_parser__create()` - Creates a new parser instance
- `ar_send_instruction_parser__destroy()` - Destroys a parser instance
- `ar_send_instruction_parser__parse()` - Parses a send instruction
- `ar_send_instruction_parser__get_error()` - Returns last error message
- `ar_send_instruction_parser__get_error_position()` - Returns error position

## Usage Example

```c
// Create parser
ar_send_instruction_parser_t *parser = ar_send_instruction_parser__create();

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
    const char *error = ar_send_instruction_parser__get_error(parser);
    size_t pos = ar_send_instruction_parser__get_error_position(parser);
    printf("Parse error at position %zu: %s\n", pos, error);
}

// Clean up
ar__instruction_ast__destroy(ast);
ar__instruction_ast__destroy(ast2);
ar_send_instruction_parser__destroy(parser);
```

## Supported Syntax

The parser handles:
- Basic send: `send(agent_id, message)`
- Complex expressions: `send(memory.agent_id, "Count: " + memory.count)`
- Nested function calls: `send(0, build("Hello {0}", memory.name))`
- Result assignment: `memory.result := send(1, "Query")`

## Error Handling

The parser detects and reports:
- Missing or invalid function name
- Missing parentheses
- Invalid argument count
- Empty or malformed arguments
- Syntax errors in expressions

## Memory Management

- Parser instance owns its error message
- Caller owns returned AST nodes
- Arguments are properly cleaned up on parse failure
- Parser can be reused for multiple parse operations

## Dependencies

- `agerun_instruction_ast.h` - AST node creation
- `agerun_heap.h` - Memory tracking
- `agerun_string.h` - String utilities
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