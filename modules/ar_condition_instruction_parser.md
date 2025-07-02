# Condition Instruction Parser Module

## Overview

The condition instruction parser module is responsible for parsing conditional (if) instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for if() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only conditional instructions.

## Interface

### Types

- `ar_condition_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_condition_instruction_parser__create(ar_log_t *ref_log)` - Create a new parser instance
- `ar_condition_instruction_parser__destroy()` - Destroy a parser instance
- `ar_condition_instruction_parser__parse()` - Parse a condition instruction
- `ar_condition_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_condition_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Usage Example

```c
// Create parser (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_condition_instruction_parser_t *parser = ar_condition_instruction_parser__create(own_log);

// Parse simple if
ar_instruction_ast_t *ast1 = ar_condition_instruction_parser__parse(
    parser, 
    "if(x > 5, \"high\", \"low\")", 
    NULL
);

// Parse if with assignment
ar_instruction_ast_t *ast2 = ar_condition_instruction_parser__parse(
    parser,
    "memory.result := if(memory.age >= 18, \"adult\", \"minor\")",
    "memory.result"
);

// Check for errors
if (!ast2) {
    // Parse errors are reported through the ar_log instance
    // The get_error() and get_error_position() functions are deprecated
    printf("Parse error occurred\n");
}

// Clean up
ar__instruction_ast__destroy(ast1);
ar__instruction_ast__destroy(ast2);
ar_condition_instruction_parser__destroy(parser);
ar_log__destroy(own_log); // Only if log was created
```

## Syntax Supported

The parser handles the if() function with this syntax:
- `if(condition, then_value, else_value)`
- Can be used with assignment: `memory.path := if(...)`

The parser correctly handles:
- Nested expressions in arguments
- Quoted strings with escapes
- Nested function calls
- Complex boolean conditions
- Whitespace variations

## Implementation Details

The parser:
1. Validates the function name is "if"
2. Ensures opening parenthesis follows
3. Extracts exactly 3 arguments (condition, then, else)
4. Handles nested parentheses and quoted strings in arguments
5. Creates an AR_INST__IF node with the parsed arguments
6. Tracks optional result assignment path

## Error Handling

The parser reports errors through the ar_log instance provided during creation:
- Errors include position and description information
- Validation of argument count
- The deprecated get_error() and get_error_position() functions always return NULL and 0

## Memory Management

- Parser instance must be destroyed when no longer needed
- Returned AST nodes are owned by caller and must be destroyed
- Errors are reported through ar_log (deprecated get_error functions return NULL/0)
- All internal allocations are properly tracked with heap macros

## Integration

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only conditional instructions.