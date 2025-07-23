# Build Instruction Evaluator Module

## Overview

The build instruction evaluator module is responsible for evaluating build instructions in the AgeRun language. It constructs strings from templates by substituting placeholder values.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

**Implementation Note**: This module is implemented in Zig for improved memory safety and cleaner error handling through defer statements.

## Purpose

This module extracts the build instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for template-based string construction.

## Key Components

### Types

```c
typedef struct ar_build_instruction_evaluator_s ar_build_instruction_evaluator_t;
```

An opaque type representing a build instruction evaluator instance.

### Public Interface

```c
ar_build_instruction_evaluator_t* ar_build_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);
```
Creates a new build instruction evaluator that stores its dependencies including the log for error reporting. Uses frame-based execution pattern.

```c
void ar_build_instruction_evaluator__destroy(
    ar_build_instruction_evaluator_t *own_evaluator
);
```
Destroys a build instruction evaluator and frees all resources.

```c
bool ar_build_instruction_evaluator__evaluate(
    const ar_build_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates a build instruction using the stored dependencies. Takes a frame containing memory and context for frame-based execution. The evaluator parameter is now const for improved API safety.


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
- Expression evaluator and log are borrowed references stored in the instance
- Memory is obtained from the frame parameter during evaluation
- Template and values map evaluations are temporary
- Values are accessed as references from the map
- Result string is created with proper ownership
- All intermediate allocations properly freed
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_frame`: For frame-based execution context
- `ar_data`: For data manipulation
- `ar_heap`: For memory tracking

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
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create build instruction evaluator (frame-based pattern)
ar_build_instruction_evaluator_t *build_eval = ar_build_instruction_evaluator__create(
    log, expr_eval
);

// Set up values in memory
ar_data_t *values = ar_data__create_map();
ar_data__set_map_data(values, "name", ar_data__create_string("World"));
ar_data__set_map_data(values, "count", ar_data__create_integer(42));
ar_data__set_map_data(memory, "values", values);

// Create frame for execution
ar_frame_t *frame = ar_frame__create(memory, NULL);

// Parse build instruction: memory.msg := build("Hello {name}! Count: {count}", memory.values)
ar_instruction_ast_t *ast = ar_instruction_parser__parse_build(parser);

// Evaluate the build with frame
bool success = ar_build_instruction_evaluator__evaluate(build_eval, frame, ast);

// memory["msg"] now contains "Hello World! Count: 42"

// Cleanup
ar_build_instruction_evaluator__destroy(build_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_frame__destroy(frame);
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

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.