# Parse Instruction Evaluator Module

## Overview

The parse instruction evaluator module is responsible for evaluating parse instructions in the AgeRun language. It extracts values from strings using template patterns with placeholders.

## Purpose

This module extracts the parse instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for template-based string parsing and value extraction.

## Key Components

### Public Interface

```c
bool ar_parse_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);
```

### Functionality

The module evaluates parse instructions of the form:
- `memory.result := parse(template, input_string)`

Key features:
1. **Template Patterns**: Supports templates with `{name}` placeholders
2. **Value Extraction**: Extracts substrings matching placeholder positions
3. **Multiple Placeholders**: Can extract multiple values from one string
4. **Result Storage**: Stores extracted values as a map in memory

### Template Syntax

Templates use curly braces to mark placeholders:
- `"Hello {name}"` - Extracts the name portion
- `"{greeting} {name}!"` - Extracts both greeting and name
- `"Value: {num}"` - Extracts the numeric portion as a string

### Memory Management

The module follows strict memory ownership rules:
- Template and input string evaluations are owned temporarily
- Extracted values are stored in a new map
- Map ownership is transferred to memory storage
- All intermediate values properly cleaned up

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions
- `agerun_expression_parser`: For parsing expression strings
- `agerun_expression_ast`: For expression AST nodes
- `agerun_instruction_ast`: For accessing instruction AST structure
- `agerun_instruction`: For parse operation implementation
- `agerun_data`: For data manipulation
- `agerun_string`: For string operations
- `agerun_heap`: For memory tracking

## Implementation Details

The module:
1. Evaluates template and input string expressions
2. Validates both are strings
3. Calls `ar__instruction__parse()` to perform extraction
4. Stores resulting map in memory at specified path
5. Handles all error cases with proper cleanup

## Usage Example

```c
// Create evaluator
expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Parse instruction: memory.data := parse("Hello {name}!", "Hello World!")
instruction_ast_t *ast = ar__instruction_parser__parse_parse(parser);

// Evaluate the parse
bool success = ar_parse_instruction_evaluator__evaluate(
    expr_eval, memory, ast
);

// memory["data"]["name"] now contains "World"
```

## Testing

The module includes comprehensive tests covering:
- Single placeholder extraction
- Multiple placeholder extraction
- Templates with literal text
- Edge cases (empty strings, no placeholders)
- Invalid arguments handling
- Memory leak verification

All tests pass with zero memory leaks.