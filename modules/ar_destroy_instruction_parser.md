# Destroy Instruction Parser Module

## Overview

The `ar_destroy_instruction_parser` module provides specialized parsing for `destroy()` function instructions with a single argument in the AgeRun language. This parser handles destroy instructions that target agents by their ID.

## Features

- **Instantiable Design**: Create parser instances with `ar_destroy_instruction_parser__create(ar_log_t *ref_log)` and destroy with `ar_destroy_instruction_parser__destroy()`
- **Single Argument Parsing**: Specifically handles `destroy(agent_id)` syntax
- **Error Handling**: Comprehensive error reporting with specific error messages and position tracking
- **Memory Safety**: Zero memory leaks with proper ownership management following MMM guidelines
- **Assignment Support**: Handles both direct calls and assignment forms (e.g., `memory.result := destroy(agent_id)`)

## Usage

### Basic Usage

```c
#include "ar_destroy_instruction_parser.h"

// Create parser instance (with optional logging)
ar_log_t *own_log = ar_log__create(); // Optional - can be NULL
ar_destroy_instruction_parser_t *parser = ar_destroy_instruction_parser__create(own_log);

// Parse destroy agent instruction
const char *instruction = "destroy(123)";
ar_instruction_ast_t *ast = ar_destroy_instruction_parser__parse(parser, instruction, NULL);

if (ast) {
    // Successfully parsed
    assert(ar_instruction_ast__get_type(ast) == AR_INSTRUCTION_AST_TYPE__DESTROY);
    // ... use AST ...
    ar_instruction_ast__destroy(ast);
} else {
    // Parse errors are reported through the ar_log instance
    // The get_error() and get_error_position() functions are deprecated
    printf("Parse error occurred\n");
}

// Cleanup
ar_destroy_instruction_parser__destroy(parser);
ar_log__destroy(own_log); // Only if log was created
```

### Assignment Form

```c
// Parse destroy instruction with assignment
const char *instruction = "memory.result := destroy(memory.agent_id)";
ar_instruction_ast_t *ast = ar_destroy_instruction_parser__parse(parser, instruction, "memory.result");

if (ast) {
    assert(ar_instruction_ast__has_result_assignment(ast) == true);
    // ... use AST ...
}
```

## Supported Forms

### Integer Agent ID
```
destroy(123)
```

### Memory Reference
```
destroy(memory.agent_id)
```

### Expression
```
destroy(memory.agents[0])
```

### Assignment Forms
All forms support assignment:
```
memory.result := destroy(123)
memory.success := destroy(memory.agent_id)
```

## Error Handling

The parser reports errors through the ar_log instance provided during creation. Common issues include:

- **Missing parentheses**: `"Expected '(' after 'destroy'"`
- **Wrong function name**: `"Expected 'destroy' function"`
- **Empty arguments**: `"Failed to parse destroy argument"`
- **Memory allocation failures**: `"Memory allocation failed"`

Access error information using:
- `ar_destroy_instruction_parser__get_error()` - DEPRECATED: Always returns NULL. Use ar_log for error reporting
- `ar_destroy_instruction_parser__get_error_position()` - DEPRECATED: Always returns 0. Error positions are reported through ar_log

## Implementation Details

### Architecture
- **Opaque Type**: `ar_destroy_instruction_parser_t` hides implementation details
- **Error State**: Reports errors through ar_log (deprecated get_error functions return NULL/0)
- **Instance-Based**: Each parser instance maintains its own state
- **AST Type**: Creates `AR_INSTRUCTION_AST_TYPE__DESTROY` nodes

### Memory Management
- **Heap Tracking**: Uses `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
- **Ownership Naming**: Follows `own_`, `mut_`, `ref_` prefixes consistently
- **Zero Leaks**: All allocations are properly tracked and freed

### Argument Parsing
The parser uses `_extract_argument()` to parse a single argument up to the closing parenthesis. It handles:
- Nested parentheses in expressions
- Quoted strings
- Whitespace trimming
- Empty argument detection

## Dependencies

- `ar_instruction_ast.h` - For creating AST nodes
- `ar_heap.h` - For memory tracking
- Standard C libraries: `string.h`, `stdio.h`, `ctype.h`, `stdbool.h`

## Testing

The module includes comprehensive tests covering:
- Create/destroy lifecycle
- Integer agent ID parsing
- Memory reference parsing
- Assignment forms
- Error handling for invalid inputs

Run tests with:
```bash
make bin/ar_destroy_instruction_parser_tests
```

## Integration

This module is part of the specialized parser architecture where the main `instruction_parser` acts as a facade coordinating multiple specialized parsers. It will be integrated into the main parser once all specialized parsers are complete.

## Relationship to Destroy Method Parser

This parser handles single-argument destroy instructions targeting agents. The companion `destroy_method_instruction_parser` handles two-argument destroy instructions targeting methods. The instruction evaluator will dispatch to the appropriate destroy evaluator based on the AST type.