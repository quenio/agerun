# Destroy Agent Instruction Evaluator Module

## Overview

The destroy agent instruction evaluator module is responsible for evaluating agent destruction instructions in the AgeRun language. It handles the `destroy(agent_id)` instruction form, which destroys a specific agent by its ID.

## Purpose

This module extracts the agent destruction logic from the main destroy instruction evaluator, following the single responsibility principle. It provides specialized handling for agent destruction with proper result storage.

## Key Components

### Instantiable Design

The module follows an instantiable design pattern with lifecycle management:

```c
// Create evaluator instance with dependencies
ar_destroy_agent_instruction_evaluator_t* ar_destroy_agent_instruction_evaluator__create(
    ar_expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
);

// Evaluate using stored dependencies
bool ar_destroy_agent_instruction_evaluator__evaluate(
    const ar_destroy_agent_instruction_evaluator_t *ref_evaluator,
    const ar_instruction_ast_t *ref_ast
);

// Clean up instance
void ar_destroy_agent_instruction_evaluator__destroy(ar_destroy_agent_instruction_evaluator_t *own_evaluator);
```

### Legacy Interface (Backward Compatibility)

```c
```

### Functionality

The module evaluates destroy agent instructions of the form:
- `destroy(agent_id)`
- `memory.result := destroy(agent_id)`

Key features:
1. **Agent ID Evaluation**: Evaluates the agent ID expression to an integer
2. **Agent Destruction**: Destroys the agent if it exists
3. **Result Assignment**: Stores true (1) if agent was destroyed, false (0) otherwise
4. **Error Handling**: Returns false for invalid argument types

### Memory Management

The module follows strict memory ownership rules:
- Agent ID evaluation creates temporary data that is properly cleaned up
- Result value is created and transferred to memory when assignment specified
- All temporary values properly destroyed

## Dependencies

- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_agency`: For agent destruction
- `ar_data`: For data manipulation
- `ar_heap`: For memory tracking

## Implementation Details

The module:
1. Validates argument count (must be exactly 1)
2. Evaluates the agent ID expression
3. Validates the result is an integer
4. Calls agency to destroy the agent
5. Stores result if assignment specified

## Usage Examples

### Modern Instance-Based Approach (Recommended)

```c
// Create dependencies
data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create destroy agent evaluator instance
ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
    expr_eval, memory
);

// Parse destroy instruction: memory.result := destroy(42)
ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
    AR_INST__DESTROY, "destroy", args, 1, "memory.result"
);

// Evaluate using instance
bool success = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);

// Clean up
ar_destroy_agent_instruction_evaluator__destroy(evaluator);
// Result stored in memory["result"]: 1 if destroyed, 0 if not found
```

### Legacy Approach (Backward Compatibility)

```c
```

## Testing

The module includes comprehensive tests covering:

### Instance-Based Interface Tests
- Create/destroy lifecycle functions
- Instance-based evaluation using stored dependencies
- Legacy function backward compatibility

### Functional Tests
- Destroy existing agent
- Destroy with result assignment
- Destroy nonexistent agent
- Invalid agent ID type
- Wrong number of arguments
- Memory leak verification

All tests pass with zero memory leaks (293 allocations, 0 active).