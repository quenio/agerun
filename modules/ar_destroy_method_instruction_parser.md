# Destroy Method Instruction Parser Module

## Overview

The `ar_destroy_method_instruction_parser` module provides specialized parsing for `destroy()` function instructions with two string arguments in the AgeRun language. This parser handles destroy instructions that target methods by their name and version.

## Features

- **Instantiable Design**: Create parser instances with `ar_destroy_method_instruction_parser__create(ar_log_t *ref_log)` and destroy with `ar_destroy_method_instruction_parser__destroy()`
- **Two Argument Parsing**: Specifically handles `destroy("method_name", "version")` syntax
- **Error Handling**: Comprehensive error reporting with specific error messages and position tracking
- **Memory Safety**: Zero memory leaks with proper ownership management following MMM guidelines
- **Assignment Support**: Handles both direct calls and assignment forms (e.g., `memory.result := destroy("method", "version")`)

## Usage

### Basic Usage

```c
#include "ar_destroy_method_instruction_parser.h"

// Create parser instance (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_destroy_method_instruction_parser_t *parser = ar_destroy_method_instruction_parser__create(own_log);

// Parse destroy method instruction
const char *instruction = "destroy(\"calculator\", \"1.0.0\")";
ar_instruction_ast_t *ast = ar_destroy_method_instruction_parser__parse(parser, instruction, NULL);

if (ast) {
    // Successfully parsed
    assert(ar_instruction_ast__get_type(ast) == AR_INST__DESTROY_METHOD);
    // ... use AST ...
    ar_instruction_ast__destroy(ast);
} else {
    // Parse errors are reported through the ar_log instance
    // The get_error() and get_error_position() functions are deprecated
    printf("Parse error occurred\n");
}

// Cleanup
ar_destroy_method_instruction_parser__destroy(parser);
ar_log__destroy(own_log); // Only if log was created
```

### Assignment Form

```c
// Parse destroy instruction with assignment
const char *instruction = "memory.result := destroy(\"test_method\", \"2.0.0\")";
ar_instruction_ast_t *ast = ar_destroy_method_instruction_parser__parse(parser, instruction, "memory.result");

if (ast) {
    assert(ar_instruction_ast__has_result_assignment(ast) == true);
    // ... use AST ...
}
```

## Supported Forms

### Basic Form
```
destroy("method_name", "version")
```

### With Assignment
```
memory.result := destroy("method_name", "version")
memory.success := destroy("calculator", "1.0.0")
```

### Complex Strings
Handles escaped quotes and special characters:
```
destroy("test\"method", "1.0.0-beta")
destroy("multi\nline", "1.0.0")
```

## Error Handling

The parser reports errors through the ar_log instance provided during creation. Common issues include:

- **Missing parentheses**: `"Expected '(' after 'destroy'"`
- **Wrong function name**: `"Expected 'destroy' function"`
- **Missing second argument**: `"Failed to parse method name argument"` (when only one arg provided)
- **Invalid version argument**: `"Failed to parse version argument"`
- **Memory allocation failures**: `"Memory allocation failed"`

Access error information using:
- `ar_destroy_method_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_destroy_method_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Implementation Details

### Architecture
- **Opaque Type**: `ar_destroy_method_instruction_parser_t` hides implementation details
- **Error State**: Reports errors through ar_log (deprecated get_error functions return NULL/0)
- **Instance-Based**: Each parser instance maintains its own state
- **AST Type**: Creates `AR_INST__DESTROY_METHOD` nodes

### Memory Management
- **Heap Tracking**: Uses `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
- **Ownership Naming**: Follows `own_`, `mut_`, `ref_` prefixes consistently
- **Zero Leaks**: All allocations are properly tracked and freed

### Argument Parsing
The parser uses `_extract_argument()` to parse two arguments:
1. First argument up to comma delimiter
2. Second argument up to closing parenthesis

Both arguments are expected to be quoted strings for method destruction, though the parser itself doesn't validate string format (that's done by the evaluator).

## Validation

The parser performs syntactic validation:
- Requires exactly 2 arguments separated by comma
- Checks for proper parentheses and delimiters

Semantic validation (e.g., arguments must be quoted strings) is performed by the destroy method instruction evaluator.

## Dependencies

- `ar_instruction_ast.h` - For creating AST nodes
- `ar_heap.h` - For memory tracking
- Standard C libraries: `string.h`, `stdio.h`, `ctype.h`, `stdbool.h`

## Testing

The module includes comprehensive tests covering:
- Create/destroy lifecycle
- Two string argument parsing
- Assignment forms
- Error handling for various invalid inputs
- Complex string handling with escaped quotes

Run tests with:
```bash
make bin/ar_destroy_method_instruction_parser_tests
```

## Integration

This module is part of the specialized parser architecture where the main `instruction_parser` acts as a facade coordinating multiple specialized parsers. It will be integrated into the main parser once all specialized parsers are complete.

## Relationship to Destroy Agent Parser

This parser handles two-argument destroy instructions targeting methods. The companion `destroy_agent_instruction_parser` handles single-argument destroy instructions targeting agents. The instruction evaluator will dispatch to the appropriate destroy evaluator based on the AST type.