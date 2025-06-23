# Build Instruction Parser Module

## Overview

The build instruction parser module is responsible for parsing build() function instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for build() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only build instructions.

## Interface

### Types

- `ar_build_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_build_instruction_parser__create()` - Create a new parser instance
- `ar_build_instruction_parser__destroy()` - Destroy a parser instance
- `ar_build_instruction_parser__parse()` - Parse a build instruction
- `ar_build_instruction_parser__get_error()` - Get error message from last parse
- `ar_build_instruction_parser__get_error_position()` - Get error position

## Usage Example

```c
// Create parser
ar_build_instruction_parser_t *parser = ar_build_instruction_parser__create();

// Parse simple build function
ar_instruction_ast_t *ast1 = ar_build_instruction_parser__parse(
    parser, 
    "build(\"Hello {name}!\", memory.data)", 
    NULL
);

// Parse with assignment
ar_instruction_ast_t *ast2 = ar_build_instruction_parser__parse(
    parser,
    "memory.greeting := build(\"User: {firstName} {lastName}\", memory.user)",
    "memory.greeting"
);

// Check for errors
if (!ast2) {
    const char *error = ar_build_instruction_parser__get_error(parser);
    size_t pos = ar_build_instruction_parser__get_error_position(parser);
    printf("Parse error at position %zu: %s\n", pos, error);
}

// Clean up
ar__instruction_ast__destroy(ast1);
ar__instruction_ast__destroy(ast2);
ar_build_instruction_parser__destroy(parser);
```

## Syntax Supported

The parser handles the build() function with this syntax:
- `build(template, map)` - Combines template with values from map
- Template uses `{variable}` placeholders
- Map must be an expression evaluating to a map value
- Can be used with assignment: `memory.path := build(...)`

The parser correctly handles:
- Quoted string templates with placeholders
- Multiple placeholders in one template
- Escaped quotes within templates
- Whitespace variations
- Memory references as map arguments

## Implementation Details

The parser:
1. Validates the function name is "build"
2. Ensures opening parenthesis follows
3. Extracts exactly 2 arguments (template string and map expression)
4. Handles quoted strings with escape sequences
5. Creates an AR_INST__BUILD node with the parsed arguments
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

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only build() function instructions.

## Build Function Behavior

The build() function is used to combine template strings with values from maps:
- Template pattern: `"Hello {name}, you are {age} years old"`
- Map values: `{"name": "Alice", "age": 30}`
- Result: `"Hello Alice, you are 30 years old"`

This parser only handles the syntax parsing. The actual template processing and value substitution is performed by the build_instruction_evaluator module.