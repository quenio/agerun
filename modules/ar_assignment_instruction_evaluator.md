# Assignment Instruction Evaluator Module

## Overview

The assignment instruction evaluator module is responsible for evaluating assignment instructions in the AgeRun language. It handles storing values into memory using dot-notation paths.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the assignment instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for memory assignments with support for nested paths.

## Key Components

### Types

```c
typedef struct ar_assignment_instruction_evaluator_s assignment_instruction_evaluator_t;
```

An opaque type representing an assignment instruction evaluator instance.

### Public Interface

```c
assignment_instruction_evaluator_t* ar_assignment_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);
```
Creates a new assignment instruction evaluator that stores its dependencies.

```c
void ar_assignment_instruction_evaluator__destroy(
    assignment_instruction_evaluator_t *own_evaluator
);
```
Destroys an assignment instruction evaluator and frees all resources.

```c
bool ar_assignment_instruction_evaluator__evaluate(
    assignment_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates an assignment instruction using the stored dependencies.


### Functionality

The module evaluates assignment instructions of the form:
- `memory.key := expression`
- `memory.nested.path := expression`

Key features:
1. **Path Validation**: Ensures assignment paths start with "memory."
2. **Expression Evaluation**: Evaluates the right-hand side expression
3. **Nested Path Support**: Creates intermediate maps as needed for nested paths
4. **Ownership Transfer**: Properly transfers ownership of evaluated values to memory

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator and memory are borrowed references stored in the instance
- Expression evaluation results are owned and must be stored or destroyed
- Intermediate maps are created as needed for nested paths
- All allocated memory is properly managed with no leaks
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

The module includes helper functions for:
- Parsing and evaluating expression strings
- Extracting memory key paths from assignment targets
- Storing values in memory with proper ownership transfer

## Usage Example

```c
// Create memory and expression evaluator
data_t *memory = ar__data__create_map();
ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Create assignment instruction evaluator
assignment_instruction_evaluator_t *assign_eval = ar_assignment_instruction_evaluator__create(
    expr_eval, memory
);

// Parse assignment instruction: memory.result := 42
ar_instruction_ast_t *ast = ar__instruction_parser__parse_assignment(parser);

// Evaluate the assignment
bool success = ar_assignment_instruction_evaluator__evaluate(assign_eval, ast);

// The value 42 is now stored at memory["result"]

// Cleanup
ar_assignment_instruction_evaluator__destroy(assign_eval);
ar__expression_evaluator__destroy(expr_eval);
ar__data__destroy(memory);
```

## Testing

The module includes comprehensive tests covering:
- Simple assignments
- Nested path assignments
- Various expression types (literals, operations, memory access)
- Error handling for invalid paths
- Memory leak verification

All tests pass with zero memory leaks.