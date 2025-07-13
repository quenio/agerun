# Deprecate Instruction Evaluator Module

## Overview

The deprecate instruction evaluator module is responsible for evaluating method deprecation instructions in the AgeRun language. It handles the `deprecate(method_name, version)` instruction form, which deprecates a specific method and destroys any agents using it.

## Purpose

This module extracts the method deprecation logic from the main destroy instruction evaluator, following the single responsibility principle. It provides specialized handling for method deprecation with proper agent lifecycle management.

## Key Components

### Instantiable Design

The module follows an instantiable design pattern with lifecycle management:

```c
// Create evaluator instance with dependencies
ar_deprecate_instruction_evaluator_t* ar_deprecate_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory
);

// Evaluate using stored dependencies
bool ar_deprecate_instruction_evaluator__evaluate(
    ar_deprecate_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);

// Clean up instance
void ar_deprecate_instruction_evaluator__destroy(ar_deprecate_instruction_evaluator_t *own_evaluator);
```

### Frame-Based Execution

The module has been fully migrated to the frame-based execution pattern, which bundles memory, context, and message into a single frame parameter. This eliminates the need to store memory references in the evaluator instance and provides a clean, stateless evaluation pattern consistent with other instruction evaluators.

### Functionality

The module evaluates deprecate instructions of the form:
- `deprecate("method_name", "version")`
- `memory.result := deprecate("method_name", "version")`

Key features:
1. **Frame-Based Execution**: Uses ar_frame_t for memory, context, and message bundling
2. **Method Name/Version Evaluation**: Evaluates both arguments to strings
3. **Agent Lifecycle Management**: Sends `__sleep__` messages to agents before destruction
4. **Agent Destruction**: Destroys all agents using the method
5. **Method Unregistration**: Removes the method from methodology
6. **Result Assignment**: Stores true (1) if method was destroyed, false (0) otherwise
7. **Memory Access**: Gets memory from frame during evaluation

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
- Expression evaluator and log are borrowed references stored in the instance
- Memory is accessed from frame during evaluation, not stored in evaluator
- Method name and version evaluations create temporary data that is properly cleaned up
- Sleep messages are created and ownership transferred via send
- Result value is created and transferred to memory when assignment specified
- All temporary values properly destroyed
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_frame`: For frame-based execution context
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

### Frame-Based Approach (Recommended)

```c
// Create dependencies
ar_data_t *memory = ar_data__create_map();
ar_log_t *log = ar_log__create();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);

// Create destroy method evaluator instance (frame-based pattern)
ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
    log, expr_eval
);

// Parse destroy instruction: memory.result := deprecate("calculator", "1.0.0")
const char *args[] = {"\"calculator\"", "\"1.0.0\""};
ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
    AR_INSTRUCTION_AST_TYPE__DESTROY_METHOD, "destroy", args, 2, "memory.result"
);

// Create frame for evaluation
ar_data_t *context = ar_data__create_map();
ar_data_t *message = ar_data__create_string("");
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate using frame-based pattern
bool success = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);

// Clean up
ar_frame__destroy(frame);
ar_data__destroy(context);
ar_data__destroy(message);
ar_deprecate_instruction_evaluator__destroy(evaluator);
// Result stored in memory["result"]: 1 if destroyed, 0 if not found
```

## Testing

The module includes comprehensive tests covering:

### Frame-Based Interface Tests
- Create/destroy lifecycle functions
- Frame-based evaluation using instruction evaluator fixture
- Upfront creation pattern verification

### Functional Tests
- Destroy existing method
- Destroy method with agents using it
- Destroy nonexistent method
- Invalid method name type
- Wrong number of arguments
- Agent lifecycle event handling
- Memory leak verification

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.