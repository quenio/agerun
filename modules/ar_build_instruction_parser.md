# Build Instruction Parser Module

## Overview

The build instruction parser module is responsible for parsing build() function instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for build() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only build instructions.

## Interface

### Types

- `ar_build_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_build_instruction_parser__create(ar_log_t *ref_log)` - Create a new parser instance with optional logging
- `ar_build_instruction_parser__destroy()` - Destroy a parser instance
- `ar_build_instruction_parser__parse()` - Parse a build instruction
- `ar_build_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_build_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Usage Example

```c
// Create parser (with optional ar_log for error reporting)
ar_log_t *log = ar_log__create();  // Optional - can pass NULL
ar_build_instruction_parser_t *parser = ar_build_instruction_parser__create(log);

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
    // Parse errors are reported through the ar_log instance
    // The get_error() and get_error_position() functions are deprecated
    printf("Parse error occurred\n");
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

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only build() function instructions.

## Build Function Behavior

The build() function is used to combine template strings with values from maps:
- Template pattern: `"Hello {name}, you are {age} years old"`
- Map values: `{"name": "Alice", "age": 30}`
- Result: `"Hello Alice, you are 30 years old"`

This parser only handles the syntax parsing. The actual template processing and value substitution is performed by the build_instruction_evaluator module.