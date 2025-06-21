# Condition Instruction Parser Module

## Overview

The condition instruction parser module is responsible for parsing conditional (if) instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for if() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only conditional instructions.

## Interface

### Types

- `ar_condition_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_condition_instruction_parser__create()` - Create a new parser instance
- `ar_condition_instruction_parser__destroy()` - Destroy a parser instance
- `ar_condition_instruction_parser__parse()` - Parse a condition instruction
- `ar_condition_instruction_parser__get_error()` - Get error message from last parse
- `ar_condition_instruction_parser__get_error_position()` - Get error position

## Usage Example

```c
// Create parser
ar_condition_instruction_parser_t *parser = ar_condition_instruction_parser__create();

// Parse simple if
instruction_ast_t *ast1 = ar_condition_instruction_parser__parse(
    parser, 
    "if(x > 5, \"high\", \"low\")", 
    NULL
);

// Parse if with assignment
instruction_ast_t *ast2 = ar_condition_instruction_parser__parse(
    parser,
    "memory.result := if(memory.age >= 18, \"adult\", \"minor\")",
    "memory.result"
);

// Check for errors
if (!ast2) {
    const char *error = ar_condition_instruction_parser__get_error(parser);
    size_t pos = ar_condition_instruction_parser__get_error_position(parser);
    printf("Parse error at position %zu: %s\n", pos, error);
}

// Clean up
ar__instruction_ast__destroy(ast1);
ar__instruction_ast__destroy(ast2);
ar_condition_instruction_parser__destroy(parser);
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
5. Creates an INST_AST_IF node with the parsed arguments
6. Tracks optional result assignment path

## Error Handling

The parser provides detailed error messages including:
- Position where the error occurred
- Description of what was expected
- Validation of argument count

## Memory Management

- Parser instance must be destroyed when no longer needed
- Returned AST nodes are owned by caller and must be destroyed
- Error messages are owned by the parser and valid until next parse or destroy
- All internal allocations are properly tracked with heap macros

## Integration

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only conditional instructions.