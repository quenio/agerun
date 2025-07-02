# Method Instruction Parser Module

## Overview

The method instruction parser module is responsible for parsing method() function instructions in the AgeRun language. It follows the specialized parser pattern, providing an instantiable parser with create/destroy lifecycle.

## Purpose

This module extracts the parsing logic for method() function calls from the general instruction parser, creating a focused, single-responsibility component that handles only method instructions.

## Interface

### Types

- `ar_method_instruction_parser_t` - Opaque parser instance type

### Functions

- `ar_method_instruction_parser__create(ar_log_t *ref_log)` - Create a new parser instance
- `ar_method_instruction_parser__destroy()` - Destroy a parser instance
- `ar_method_instruction_parser__parse()` - Parse a method instruction
- `ar_method_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_method_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Usage Example

```c
// Create parser (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_method_instruction_parser_t *parser = ar_method_instruction_parser__create(own_log);

// Parse simple method function
ar_instruction_ast_t *ast1 = ar_method_instruction_parser__parse(
    parser, 
    "method(\"greet\", \"memory.msg := \\\"Hello\\\"\", \"1.0.0\")", 
    NULL
);

// Parse with assignment
ar_instruction_ast_t *ast2 = ar_method_instruction_parser__parse(
    parser,
    "memory.method_ref := method(\"calculate\", \"memory.result := 42\", \"2.0.0\")",
    "memory.method_ref"
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
ar_method_instruction_parser__destroy(parser);
ar_log__destroy(own_log); // Only if log was created
```

## Syntax Supported

The parser handles the method() function with this syntax:
- `method(name, code, version)` - Creates a new method with given parameters
- All three arguments must be string literals
- Can be used with assignment: `memory.path := method(...)`

The parser correctly handles:
- Method names as quoted strings
- Method code containing AgeRun instructions (with escaped quotes)
- Version strings in semantic versioning format
- Whitespace variations between tokens
- Complex code with nested function calls

## Implementation Details

The parser:
1. Validates the function name is "method"
2. Ensures opening parenthesis follows
3. Extracts exactly 3 string arguments (name, code, version)
4. Handles quoted strings with escape sequences
5. Creates an AR_INST__METHOD node with the parsed arguments
6. Tracks optional result assignment path

## Error Handling

The parser reports errors through the ar_log instance provided during creation:
- Errors include position and description information
- Validation of argument count (must be exactly 3)
- Validation that all arguments are strings
- The deprecated get_error() and get_error_position() functions always return NULL and 0

## Memory Management

- Parser instance must be destroyed when no longer needed
- Returned AST nodes are owned by caller and must be destroyed
- Errors are reported through ar_log (deprecated get_error functions return NULL/0)
- All internal allocations are properly tracked with heap macros

## Integration

This parser is designed to be used by the main instruction_parser module as part of the facade pattern for instruction parsing. It can also be used standalone for parsing only method() function instructions.

## Method Function Behavior

The method() function is used to create new methods in the system:
- Name: The method identifier used for invocation
- Code: AgeRun instructions that define the method's behavior
- Version: Semantic version string for method versioning

This parser only handles the syntax parsing. The actual method creation and registration is performed by the method_instruction_evaluator module.