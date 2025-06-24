# Parse Instruction Evaluator Module

## Overview

The parse instruction evaluator module is responsible for evaluating parse instructions in the AgeRun language. It extracts values from strings using template patterns with placeholders.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the parse instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for template-based string parsing and value extraction.

## Key Components

### Types

```c
typedef struct ar_parse_instruction_evaluator_s parse_instruction_evaluator_t;
```

An opaque type representing a parse instruction evaluator instance.

### Public Interface

```c
parse_instruction_evaluator_t* ar_parse_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);
```
Creates a new parse instruction evaluator that stores its dependencies.

```c
void ar_parse_instruction_evaluator__destroy(
    parse_instruction_evaluator_t *own_evaluator
);
```
Destroys a parse instruction evaluator and frees all resources.

```c
bool ar_parse_instruction_evaluator__evaluate(
    parse_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates a parse instruction using the stored dependencies.


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
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator and memory are borrowed references stored in the instance
- Template and input string evaluations are owned temporarily
- Extracted values are stored in a new map
- Map ownership is transferred to memory storage
- All intermediate values properly cleaned up
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_instruction`: For parse operation implementation
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module:
1. Evaluates template and input string expressions
2. Validates both are strings
3. Calls `ar__instruction__parse()` to perform extraction
4. Stores resulting map in memory at specified path
5. Handles all error cases with proper cleanup

## Usage Example

```c
// Create memory and expression evaluator
data_t *memory = ar__data__create_map();
ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Create parse instruction evaluator
parse_instruction_evaluator_t *parse_eval = ar_parse_instruction_evaluator__create(
    expr_eval, memory
);

// Parse instruction: memory.data := parse("Hello {name}!", "Hello World!")
ar_instruction_ast_t *ast = ar__instruction_parser__parse_parse(parser);

// Evaluate the parse
bool success = ar_parse_instruction_evaluator__evaluate(parse_eval, ast);

// memory["data"]["name"] now contains "World"

// Cleanup
ar_parse_instruction_evaluator__destroy(parse_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_data__destroy(memory);
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