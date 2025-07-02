# Parse Instruction Parser Module

## Overview

The parse instruction parser module is responsible for parsing parse() function instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for parse() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only parse instructions.

## Interface

### Types

- `ar_parse_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_parse_instruction_parser__create(ar_log_t *ref_log)` - Create a new parser instance
- `ar_parse_instruction_parser__destroy()` - Destroy a parser instance
- `ar_parse_instruction_parser__parse()` - Parse a parse instruction
- `ar_parse_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_parse_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Usage Example

```c
// Create parser (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_parse_instruction_parser_t *parser = ar_parse_instruction_parser__create(own_log);

// Parse simple parse function
ar_instruction_ast_t *ast1 = ar_parse_instruction_parser__parse(
    parser, 
    "parse(\"name={name}\", \"name=John\")", 
    NULL
);

// Parse with assignment
ar_instruction_ast_t *ast2 = ar_parse_instruction_parser__parse(
    parser,
    "memory.result := parse(\"template: {var}\", \"template: value\")",
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
ar_parse_instruction_parser__destroy(parser);
ar_log__destroy(own_log); // Only if log was created
```

## Syntax Supported

The parser handles the parse() function with this syntax:
- `parse(template, input)` - Extracts values from input based on template
- Template uses `{variable}` placeholders
- Can be used with assignment: `memory.path := parse(...)`

The parser correctly handles:
- Quoted string arguments
- Escaped quotes within strings
- Whitespace variations
- Complex templates with multiple placeholders

## Implementation Details

The parser:
1. Validates the function name is "parse"
2. Ensures opening parenthesis follows
3. Extracts exactly 2 arguments (template and input strings)
4. Handles quoted strings with escape sequences
5. Creates an AR_INST__PARSE node with the parsed arguments
6. Tracks optional result assignment path

## Error Handling

The parser reports errors through the ar_log instance provided during creation:
- Errors include position and description information
- Validation of argument count (must be exactly 2)
- The deprecated get_error() and get_error_position() functions always return NULL and 0

## Memory Management

- Parser instance must be destroyed when no longer needed
- Returned AST nodes are owned by caller and must be destroyed
- Errors are reported through ar_log (deprecated get_error functions return NULL/0)
- All internal allocations are properly tracked with heap macros

## Integration

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only parse() function instructions.

## Parse Function Behavior

The parse() function is used to extract values from strings based on templates:
- Template pattern: `"Hello {name}, you are {age} years old"`
- Input string: `"Hello Alice, you are 30 years old"`
- Result: A map with `{"name": "Alice", "age": 30}`

This parser only handles the syntax parsing. The actual template matching and value extraction is performed by the parse_instruction_evaluator module.