# Assignment Instruction Parser Module

## Overview

The `assignment_instruction_parser` module provides specialized parsing for memory assignment instructions in the AgeRun language. It handles instructions of the form `memory.path := expression`.

## Purpose

This module is part of the specialized parser architecture, where each instruction type has its own dedicated parser. The assignment instruction parser:

- Validates assignment syntax (memory path, := operator, expression)
- Extracts the memory path and expression components
- Creates appropriate AST nodes for assignment instructions
- Provides detailed error reporting with position information

## Interface

### Types

- **`ar_assignment_instruction_parser_t`**: Opaque parser structure that maintains parsing state and error information

### Functions

#### Creation and Destruction

- **`ar_assignment_instruction_parser__create(ar_log_t *log)`**: Create a new parser instance with error logging
- **`ar_assignment_instruction_parser__destroy()`**: Destroy a parser instance

#### Parsing

- **`ar_assignment_instruction_parser__parse()`**: Parse an assignment instruction and return AST node

#### Error Handling

- **`ar_assignment_instruction_parser__get_error()`**: Get the last error message
- **`ar_assignment_instruction_parser__get_error_position()`**: Get the position where error occurred

## Usage Example

```c
// Create log and parser
ar_log_t *log = ar_log__create();
ar_assignment_instruction_parser_t *parser = ar_assignment_instruction_parser__create(log);

// Parse assignment instruction
ar_instruction_ast_t *ast = ar_assignment_instruction_parser__parse(
    parser, 
    "memory.user.name := \"John\""
);

if (ast) {
    // Use the AST
    const char *path = ar__instruction_ast__get_assignment_path(ast);
    const char *expr = ar__instruction_ast__get_assignment_expression(ast);
    
    // Clean up
    ar__instruction_ast__destroy(ast);
} else {
    // Handle error
    const char *error = ar_assignment_instruction_parser__get_error(parser);
    size_t pos = ar_assignment_instruction_parser__get_error_position(parser);
    fprintf(stderr, "Parse error at position %zu: %s\n", pos, error);
}

// Destroy parser
ar_assignment_instruction_parser__destroy(parser);
```

## Implementation Details

### Parsing Rules

1. **Memory Path**: Must start with "memory" followed by optional dot-separated identifiers
2. **Assignment Operator**: Must be `:=` (single `=` is not valid)
3. **Expression**: Any valid expression after the operator (parsing deferred to expression parser)
4. **Whitespace**: Leading/trailing whitespace is trimmed, internal whitespace preserved

### Error Conditions

- Empty instruction
- Missing memory prefix
- Invalid assignment operator
- Missing expression after `:=`
- Memory allocation failures

### Memory Management

- Parser maintains its own error message string (owned)
- Returns owned AST nodes that caller must destroy
- All internal allocations are cleaned up on error

## Integration

This module is designed to be used by the main `instruction_parser` facade, which:

1. Analyzes instruction content to detect assignment pattern
2. Delegates to this specialized parser
3. Returns the resulting AST node to the caller

## Dependencies

- `ar_instruction_ast`: For creating assignment AST nodes
- `ar_heap`: For memory management
- Standard C library: string.h, ctype.h

## Testing

Comprehensive tests are provided in `ar_assignment_instruction_parser_tests.c` covering:

- Basic assignment parsing
- String and expression assignments
- Nested memory paths
- Error conditions
- Parser reusability
- Memory leak verification