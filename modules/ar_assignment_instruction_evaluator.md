# Assignment Instruction Evaluator Module

## Overview

The assignment instruction evaluator module is responsible for evaluating assignment instructions in the AgeRun language. It handles storing values into memory using dot-notation paths.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the assignment instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for memory assignments with support for nested paths.

## Key Components

### Types

```c
typedef struct ar_assignment_instruction_evaluator_s ar_assignment_instruction_evaluator_t;
```

An opaque type representing an assignment instruction evaluator instance.

### Public Interface

```c
ar_assignment_instruction_evaluator_t* ar_assignment_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);
```
Creates a new assignment instruction evaluator.

**Parameters:**
- `ref_log`: Log instance for error reporting (borrowed reference)
- `ref_expr_evaluator`: Expression evaluator to use (borrowed reference)

**Returns:** New evaluator instance or NULL on failure

**Note:** Memory is no longer passed at creation time. It comes from the frame during evaluation.

```c
void ar_assignment_instruction_evaluator__destroy(
    ar_assignment_instruction_evaluator_t *own_evaluator
);
```
Destroys an assignment instruction evaluator and frees all resources.

```c
bool ar_assignment_instruction_evaluator__evaluate(
    ar_assignment_instruction_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates an assignment instruction using the frame's memory.

**Parameters:**
- `mut_evaluator`: The evaluator instance (mutable reference)
- `ref_frame`: The execution frame containing memory, context, and message (borrowed reference)
- `ref_ast`: The assignment instruction AST to evaluate (borrowed reference)

**Returns:** true on success, false on failure


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
- Expression evaluator and log are borrowed references stored in the instance
- Memory is obtained from the frame during evaluation (not stored)
- Expression evaluation results are owned and must be stored or destroyed
- Intermediate maps are created as needed for nested paths
- All allocated memory is properly managed with no leaks
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module includes helper functions for:
- Parsing and evaluating expression strings
- Extracting memory key paths from assignment targets
- Storing values in memory with proper ownership transfer

## Usage Example

```c
// Create evaluator dependencies
ar_log_t *log = ar_log__create();
ar_data_t *memory = ar_data__create_map();
ar_data_t *context = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, context);

// Create assignment instruction evaluator
ar_assignment_instruction_evaluator_t *assign_eval = ar_assignment_instruction_evaluator__create(
    log, expr_eval
);

// Parse assignment instruction: memory.result := 42
ar_instruction_ast_t *ast = ar_instruction_parser__parse_assignment(parser);

// Create a frame for evaluation
ar_data_t *message = ar_data__create_string("test");
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate the assignment
bool success = ar_assignment_instruction_evaluator__evaluate(assign_eval, frame, ast);

// The value 42 is now stored at memory["result"]

// Cleanup
ar_frame__destroy(frame);
ar_data__destroy(message);
ar_assignment_instruction_evaluator__destroy(assign_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_data__destroy(context);
ar_data__destroy(memory);
ar_log__destroy(log);
```

## Testing

The module includes comprehensive tests covering:
- Simple assignments
- Nested path assignments
- Various expression types (literals, operations, memory access)
- Error handling for invalid paths (errors reported through log)
- Memory leak verification

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.