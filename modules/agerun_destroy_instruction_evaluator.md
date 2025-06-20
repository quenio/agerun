# Destroy Instruction Evaluator Module

## Overview

The destroy instruction evaluator module serves as a dispatcher for destruction operations in the AgeRun language. It determines whether to destroy an agent or a method based on the number of arguments and delegates to the appropriate specialized evaluator.

## Purpose

This module acts as the main entry point for destroy instruction evaluation, routing requests to either the destroy agent evaluator or destroy method evaluator based on argument count. This design follows the single responsibility principle by separating agent and method destruction logic into dedicated modules.

## Key Components

### Public Interface

```c
bool ar__destroy_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);
```

### Dispatcher Functionality

The module examines destroy instructions and routes them based on argument count:
1. **One argument** (`destroy(agent_id)`) - Routes to `destroy_agent_instruction_evaluator`
2. **Two arguments** (`destroy(method_name, version)`) - Routes to `destroy_method_instruction_evaluator`
3. **Other counts** - Returns false (invalid)

### Architecture

The module follows a clean dispatcher pattern:
```
destroy_instruction_evaluator (dispatcher)
    ├── destroy_agent_instruction_evaluator (1 argument)
    │   └── Handles agent destruction
    └── destroy_method_instruction_evaluator (2 arguments)
        └── Handles method destruction with agent cleanup
```

This separation provides:
- Independent testing of agent and method destruction
- Clear separation of concerns
- Easier maintenance and extension
- Reusable components

## Dependencies

- `agerun_destroy_agent_instruction_evaluator`: For agent destruction
- `agerun_destroy_method_instruction_evaluator`: For method destruction
- `agerun_list`: For argument counting
- `agerun_heap`: For memory tracking

## Implementation Details

The dispatcher implementation is straightforward:
1. Validates input parameters and AST type
2. Gets function arguments to count them
3. Based on count, delegates to appropriate evaluator
4. Returns the result from the delegated evaluator

The actual destruction logic is handled by the specialized modules, keeping this module focused solely on routing.

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