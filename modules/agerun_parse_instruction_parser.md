# Parse Instruction Parser Module

## Overview

The parse instruction parser module is responsible for parsing parse() function instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for parse() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only parse instructions.

## Interface

### Types

- `ar_ar_parse_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_parse_instruction_parser__create()` - Create a new parser instance
- `ar_parse_instruction_parser__destroy()` - Destroy a parser instance
- `ar_parse_instruction_parser__parse()` - Parse a parse instruction
- `ar_parse_instruction_parser__get_error()` - Get error message from last parse
- `ar_parse_instruction_parser__get_error_position()` - Get error position

## Usage Example

```c
// Create parser
ar_ar_parse_instruction_parser_t *parser = ar_parse_instruction_parser__create();

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
    const char *error = ar_parse_instruction_parser__get_error(parser);
    size_t pos = ar_parse_instruction_parser__get_error_position(parser);
    printf("Parse error at position %zu: %s\n", pos, error);
}

// Clean up
ar__instruction_ast__destroy(ast1);
ar__instruction_ast__destroy(ast2);
ar_parse_instruction_parser__destroy(parser);
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

The parser provides detailed error messages including:
- Position where the error occurred
- Description of what was expected
- Validation of argument count (must be exactly 2)

## Memory Management

- Parser instance must be destroyed when no longer needed
- Returned AST nodes are owned by caller and must be destroyed
- Error messages are owned by the parser and valid until next parse or destroy
- All internal allocations are properly tracked with heap macros

## Integration

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only parse() function instructions.

## Parse Function Behavior

The parse() function is used to extract values from strings based on templates:
- Template pattern: `"Hello {name}, you are {age} years old"`
- Input string: `"Hello Alice, you are 30 years old"`
- Result: A map with `{"name": "Alice", "age": 30}`

This parser only handles the syntax parsing. The actual template matching and value extraction is performed by the parse_instruction_evaluator module.