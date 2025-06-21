# Build Instruction Evaluator Module

## Overview

The build instruction evaluator module is responsible for evaluating build instructions in the AgeRun language. It constructs strings from templates by substituting placeholder values.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the build instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for template-based string construction.

## Key Components

### Types

```c
typedef struct ar_build_instruction_evaluator_s build_instruction_evaluator_t;
```

An opaque type representing a build instruction evaluator instance.

### Public Interface

```c
build_instruction_evaluator_t* ar_build_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);
```
Creates a new build instruction evaluator that stores its dependencies.

```c
void ar_build_instruction_evaluator__destroy(
    build_instruction_evaluator_t *own_evaluator
);
```
Destroys a build instruction evaluator and frees all resources.

```c
bool ar_build_instruction_evaluator__evaluate(
    build_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```
Evaluates a build instruction using the stored dependencies.


### Functionality

The module evaluates build instructions of the form:
- `memory.result := build(template, values_map)`

Key features:
1. **Template Substitution**: Replaces `{key}` placeholders with values
2. **Type Conversion**: Automatically converts all types to strings
3. **Map-based Values**: Takes values from a map data structure
4. **Missing Keys**: Leaves placeholders unchanged if key not found

### Template Syntax

Templates use curly braces for placeholders:
- `"Hello {name}"` - Substitutes the value of "name" key
- `"{greeting} {name}!"` - Substitutes multiple values
- `"Count: {count}"` - Converts numeric values to strings

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator and memory are borrowed references stored in the instance
- Template and values map evaluations are temporary
- Values are accessed as references from the map
- Result string is created with proper ownership
- All intermediate allocations properly freed
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions
- `agerun_expression_parser`: For parsing expression strings
- `agerun_expression_ast`: For expression AST nodes
- `agerun_instruction_ast`: For accessing instruction AST structure
- `agerun_data`: For data manipulation
- `agerun_string`: For string operations
- `agerun_heap`: For memory tracking

## Implementation Details

The module:
1. Evaluates template and values expressions
2. Validates template is string and values is map
3. Iterates through template finding placeholders
4. For each placeholder, looks up value in map
5. Converts values to strings and substitutes
6. Builds final string with all substitutions

## Usage Example

```c
// Create memory and expression evaluator
data_t *memory = ar__data__create_map();
expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Create build instruction evaluator
build_instruction_evaluator_t *build_eval = ar_build_instruction_evaluator__create(
    expr_eval, memory
);

// Set up values in memory
data_t *values = ar__data__create_map();
ar__data__set_map_data(values, "name", ar__data__create_string("World"));
ar__data__set_map_data(values, "count", ar__data__create_integer(42));
ar__data__set_map_data(memory, "values", values);

// Parse build instruction: memory.msg := build("Hello {name}! Count: {count}", memory.values)
instruction_ast_t *ast = ar__instruction_parser__parse_build(parser);

// Evaluate the build
bool success = ar_build_instruction_evaluator__evaluate(build_eval, ast);

// memory["msg"] now contains "Hello World! Count: 42"

// Cleanup
ar_build_instruction_evaluator__destroy(build_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_data__destroy(memory);
```

## Testing

The module includes comprehensive tests covering:
- Single placeholder substitution
- Multiple placeholder substitution
- Different value types (string, integer, double)
- Missing keys (placeholder unchanged)
- Empty templates and maps
- Memory leak verification

All tests pass with zero memory leaks.