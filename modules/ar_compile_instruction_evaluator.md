# Compile Instruction Evaluator Module

## Overview

The compile instruction evaluator module is responsible for evaluating method creation instructions in the AgeRun language. It handles the registration of new methods with their associated instruction code.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the compile instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for method creation and registration in the methodology.

## Key Components

### Types

```c
typedef struct ar_compile_instruction_evaluator_s ar_compile_instruction_evaluator_t;
```

An opaque type representing a compile instruction evaluator instance.

### Public Interface

```c
ar_compile_instruction_evaluator_t* ar_compile_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
);
```
Creates a new compile instruction evaluator that stores its dependencies including the log for error reporting.

```c
void ar_compile_instruction_evaluator__destroy(
    ar_compile_instruction_evaluator_t *own_evaluator
);
```
Destroys a compile instruction evaluator and frees all resources.

```c
bool ar_compile_instruction_evaluator__evaluate(
    ar_compile_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates a compile instruction using the stored dependencies.


### Functionality

The module evaluates compile instructions of the form:
- `compile(name, instructions, version)`

Key features:
1. **Method Creation**: Creates new method objects with name, code, and version
2. **Version Validation**: Ensures version follows semantic versioning format
3. **Method Registration**: Registers the method in the global methodology
4. **Duplicate Prevention**: Prevents registering methods with same name/version

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator, memory, and log are borrowed references stored in the instance
- Method name, instructions, and version are evaluated to owned strings
- Method creation takes ownership of the instruction string
- Method registration transfers ownership to methodology
- All temporary values properly cleaned up
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_methodology`: For method registration
- `ar_method`: For method creation
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module:
1. Extracts and evaluates three arguments (name, instructions, version)
2. Validates all arguments are strings
3. Creates a new method object
4. Registers the method with the methodology
5. Handles registration failures (e.g., duplicates)

## Usage Example

```c
// Create memory and expression evaluator
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create compile instruction evaluator
ar_compile_instruction_evaluator_t *method_eval = ar_compile_instruction_evaluator__create(
    log, expr_eval, memory
);

// Parse compile instruction: compile("echo", "send(0, message)", "1.0.0")
ar_instruction_ast_t *ast = ar_instruction_parser__parse_compile(parser);

// Evaluate the method creation
bool success = ar_compile_instruction_evaluator__evaluate(method_eval, ast);

// Method "echo" version "1.0.0" is now registered

// Cleanup
ar_compile_instruction_evaluator__destroy(method_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_data__destroy(memory);
```

## Testing

The module includes comprehensive tests covering:
- Successful method creation
- Duplicate method detection
- Invalid argument types
- Version string validation
- Method retrieval after registration
- Memory leak verification

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.