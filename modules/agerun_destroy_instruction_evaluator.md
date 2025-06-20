# Destroy Instruction Evaluator Module

## Overview

The destroy instruction evaluator module is responsible for evaluating destroy instructions in the AgeRun language. It handles both agent destruction (single argument) and method destruction with associated agents (two arguments).

## Purpose

This module extracts the destroy instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for:
- Agent destruction by ID
- Method destruction with automatic agent cleanup
- Result assignment for destroy operations

## Key Components

### Public Interface

```c
bool ar__destroy_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);
```

### Functionality

1. **Agent Destruction** (`destroy(agent_id)`):
   - Evaluates the agent ID expression
   - Validates it's an integer
   - Calls `ar__agency__destroy_agent()`
   - Returns success/failure status

2. **Method Destruction** (`destroy(method_name, method_version)`):
   - Evaluates method name and version expressions
   - Validates both are strings
   - Checks if any agents are using the method
   - Sends `__sleep__` messages to all affected agents
   - Destroys all agents using the method
   - Unregisters the method from methodology
   - Returns true if method existed, false otherwise

### Memory Management

The module follows strict memory ownership rules:
- Expression evaluation results are owned and must be destroyed
- Helper functions create deep copies when needed
- All allocated memory is properly freed on all code paths

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions
- `agerun_expression_parser`: For parsing expression strings
- `agerun_expression_ast`: For expression AST nodes
- `agerun_agency`: For agent management
- `agerun_methodology`: For method registration
- `agerun_method`: For method references
- `agerun_heap`: For memory tracking

## Implementation Details

The module includes several helper functions:
- `_parse_and_evaluate_expression()`: Parses and evaluates expression strings
- `_evaluate_expression_ast()`: Evaluates parsed expression AST nodes
- `_store_result_if_assigned()`: Stores results when assignment is specified
- `_copy_data_value()`: Creates deep copies of data values
- `_get_memory_key_path()`: Extracts memory key paths

## Usage Example

```c
// Create evaluator
expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Parse destroy instruction: destroy(42)
instruction_ast_t *ast = ar__instruction_parser__parse_destroy(parser);

// Evaluate the destroy
bool success = ar__destroy_instruction_evaluator__evaluate(
    expr_eval, memory, ast
);

// Clean up
ar__instruction_ast__destroy(ast);
```

## Testing

The module includes comprehensive tests covering:
- Agent destruction with and without result assignment
- Method destruction with and without active agents
- Invalid argument handling
- Memory leak verification

All tests pass with zero memory leaks.