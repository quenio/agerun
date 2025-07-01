# Agent Instruction Parser Module

## Overview

The `ar_agent_instruction_parser` module provides specialized parsing for `agent()` function instructions in the AgeRun language. It follows the instantiable parser pattern with create/destroy lifecycle management and supports both 2-parameter and 3-parameter forms of agent creation.

## Features

- **Instantiable Design**: Create parser instances with `ar_agent_instruction_parser__create()` and destroy with `ar_agent_instruction_parser__destroy()`
- **Flexible Arguments**: Supports both `agent(method, version)` and `agent(method, version, context)` forms
- **Error Handling**: Comprehensive error reporting with specific error messages and position tracking
- **Memory Safety**: Zero memory leaks with proper ownership management following MMM guidelines
- **Assignment Support**: Handles both direct calls and assignment forms (e.g., `memory.id := agent(...)`)

## Usage

### Basic Usage

```c
#include "ar_agent_instruction_parser.h"

// Create parser instance (with optional ar_log for error reporting)
ar_log_t *log = ar_log__create();  // Optional - can pass NULL
ar_agent_instruction_parser_t *parser = ar_agent_instruction_parser__create(log);

// Parse agent instruction with 2 parameters
const char *instruction = "agent(\"echo\", \"1.0.0\")";
ar_instruction_ast_t *ast = ar_agent_instruction_parser__parse(parser, instruction, NULL);

if (ast) {
    // Successfully parsed
    // ... use AST ...
    ar__instruction_ast__destroy(ast);
} else {
    // Parse error occurred
    const char *error = ar_agent_instruction_parser__get_error(parser);
    size_t pos = ar_agent_instruction_parser__get_error_position(parser);
    printf("Parse error at position %zu: %s\n", pos, error);
}

// Cleanup
ar_agent_instruction_parser__destroy(parser);
```

### Assignment Form

```c
// Parse agent instruction with assignment
const char *instruction = "memory.agent_id := agent(\"echo\", \"1.0.0\", memory.config)";
ar_instruction_ast_t *ast = ar_agent_instruction_parser__parse(parser, instruction, "memory.agent_id");

if (ast) {
    assert(ar__instruction_ast__has_result_assignment(ast) == true);
    // ... use AST ...
}
```

## Supported Forms

### Two Parameters (Method and Version)
```
agent("method_name", "version")
```
For this form, the parser automatically adds a `null` context as the third argument to maintain compatibility with the instruction evaluator.

### Three Parameters (Method, Version, and Context)
```
agent("method_name", "version", memory.context)
```
The context parameter can be any valid AgeRun expression that evaluates to a map.

### Assignment Forms
Both parameter forms support assignment:
```
memory.agent_id := agent("echo", "1.0.0")
memory.agent_id := agent("echo", "1.0.0", memory.config)
```

## Error Handling

The parser provides detailed error reporting for common issues:

- **Missing parentheses**: `"Expected '(' after 'agent'"`
- **Wrong function name**: `"Expected 'agent' function"`
- **Invalid arguments**: `"Failed to parse agent arguments"`
- **Memory allocation failures**: `"Memory allocation failed"`

Access error information using:
- `ar_agent_instruction_parser__get_error()` - Get error message
- `ar_agent_instruction_parser__get_error_position()` - Get error position

## Implementation Details

### Architecture
- **Opaque Type**: `ar_agent_instruction_parser_t` hides implementation details
- **Error State**: Tracks last error message and position
- **Instance-Based**: Each parser instance maintains its own state

### Memory Management
- **Heap Tracking**: Uses `AR__HEAP__MALLOC`, `AR__HEAP__FREE`, `AR__HEAP__STRDUP`
- **Ownership Naming**: Follows `own_`, `mut_`, `ref_` prefixes consistently
- **Zero Leaks**: All allocations are properly tracked and freed

### Argument Processing
For 2-parameter calls, the parser automatically adds `"null"` as the third argument to ensure compatibility with the instruction evaluator, which expects exactly 3 arguments for agent instructions.

## Dependencies

- `ar_instruction_ast.h` - For creating AST nodes
- `ar_heap.h` - For memory tracking
- Standard C libraries: `string.h`, `stdio.h`, `ctype.h`, `stdbool.h`

## Testing

The module includes comprehensive tests covering:
- Create/destroy lifecycle
- 2-parameter parsing
- 3-parameter parsing
- Assignment forms
- Error handling for invalid inputs

Run tests with:
```bash
make bin/ar_agent_instruction_parser_tests
```

## Integration

This module is part of the specialized parser architecture where the main `instruction_parser` acts as a facade coordinating multiple specialized parsers. It will be integrated into the main parser once all specialized parsers are complete.