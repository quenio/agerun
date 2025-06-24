# Method Instruction Evaluator Module

## Overview

The method instruction evaluator module is responsible for evaluating method creation instructions in the AgeRun language. It handles the registration of new methods with their associated instruction code.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the method instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for method creation and registration in the methodology.

## Key Components

### Types

```c
typedef struct ar_method_instruction_evaluator_s ar_method_instruction_evaluator_t;
```

An opaque type representing a method instruction evaluator instance.

### Public Interface

```c
ar_method_instruction_evaluator_t* ar_method_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);
```
Creates a new method instruction evaluator that stores its dependencies.

```c
void ar_method_instruction_evaluator__destroy(
    ar_method_instruction_evaluator_t *own_evaluator
);
```
Destroys a method instruction evaluator and frees all resources.

```c
bool ar_method_instruction_evaluator__evaluate(
    ar_method_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates a method instruction using the stored dependencies.


### Functionality

The module evaluates method instructions of the form:
- `method(name, instructions, version)`

Key features:
1. **Method Creation**: Creates new method objects with name, code, and version
2. **Version Validation**: Ensures version follows semantic versioning format
3. **Method Registration**: Registers the method in the global methodology
4. **Duplicate Prevention**: Prevents registering methods with same name/version

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator and memory are borrowed references stored in the instance
- Method name, instructions, and version are evaluated to owned strings
- Method creation takes ownership of the instruction string
- Method registration transfers ownership to methodology
- All temporary values properly cleaned up
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

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
data_t *memory = ar__data__create_map();
ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Create method instruction evaluator
ar_method_instruction_evaluator_t *method_eval = ar_method_instruction_evaluator__create(
    expr_eval, memory
);

// Parse method instruction: method("echo", "send(0, message)", "1.0.0")
ar_instruction_ast_t *ast = ar__instruction_parser__parse_method(parser);

// Evaluate the method creation
bool success = ar_method_instruction_evaluator__evaluate(method_eval, ast);

// Method "echo" version "1.0.0" is now registered

// Cleanup
ar_method_instruction_evaluator__destroy(method_eval);
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

All tests pass with zero memory leaks.