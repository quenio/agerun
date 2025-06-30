# Destroy Method Instruction Evaluator Module

## Overview

The destroy method instruction evaluator module is responsible for evaluating method destruction instructions in the AgeRun language. It handles the `destroy(method_name, version)` instruction form, which destroys a specific method and any agents using it.

## Purpose

This module extracts the method destruction logic from the main destroy instruction evaluator, following the single responsibility principle. It provides specialized handling for method destruction with proper agent lifecycle management.

## Key Components

### Instantiable Design

The module follows an instantiable design pattern with lifecycle management:

```c
// Create evaluator instance with dependencies
ar_destroy_method_instruction_evaluator_t* ar_destroy_method_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
);

// Evaluate using stored dependencies
bool ar_destroy_method_instruction_evaluator__evaluate(
    ar_destroy_method_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);

// Clean up instance
void ar_destroy_method_instruction_evaluator__destroy(ar_destroy_method_instruction_evaluator_t *own_evaluator);
```

### Legacy Interface (Backward Compatibility)

```c
```

### Functionality

The module evaluates destroy method instructions of the form:
- `destroy("method_name", "version")`
- `memory.result := destroy("method_name", "version")`

Key features:
1. **Method Name/Version Evaluation**: Evaluates both arguments to strings
2. **Agent Lifecycle Management**: Sends `__sleep__` messages to agents before destruction
3. **Agent Destruction**: Destroys all agents using the method
4. **Method Unregistration**: Removes the method from methodology
5. **Result Assignment**: Stores true (1) if method was destroyed, false (0) otherwise

### Agent Lifecycle Handling

When destroying a method that has active agents:
1. Count all agents using the method
2. Send `__sleep__` message to each agent
3. Destroy each agent
4. Unregister the method

This ensures proper cleanup and lifecycle event handling.

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator, memory, and log are borrowed references stored in the instance
- Method name and version evaluations create temporary data
- Sleep messages are created and ownership transferred via send
- Result value is created and transferred to memory when assignment specified
- All temporary values properly destroyed
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_agency`: For agent operations
- `ar_methodology`: For method operations
- `ar_method`: For method references
- `ar_data`: For data manipulation
- `ar_heap`: For memory tracking

## Implementation Details

The module:
1. Validates argument count (must be exactly 2)
2. Evaluates method name and version expressions
3. Validates both results are strings
4. Checks if method exists
5. If agents are using the method:
   - Sends `__sleep__` messages to all agents
   - Destroys all agents using the method
6. Unregisters the method from methodology
7. Stores result if assignment specified

## Usage Examples

### Modern Instance-Based Approach (Recommended)

```c
// Create dependencies
data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create destroy method evaluator instance
ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
    log, expr_eval, memory
);

// Parse destroy instruction: memory.result := destroy("calculator", "1.0.0")
const char *args[] = {"\"calculator\"", "\"1.0.0\""};
ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
    AR_INST__DESTROY, "destroy", args, 2, "memory.result"
);

// Evaluate using instance
bool success = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);

// Clean up
ar_destroy_method_instruction_evaluator__destroy(evaluator);
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
- Destroy existing method
- Destroy method with agents using it
- Destroy nonexistent method
- Invalid method name type
- Wrong number of arguments
- Agent lifecycle event handling
- Memory leak verification

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.