# Exit Instruction Evaluator Module

## Overview

The exit instruction evaluator module is responsible for evaluating agent destruction instructions in the AgeRun language. It handles the `exit(agent_id)` instruction form, which terminates a specific agent by its ID.

**Implementation Note**: This module has been migrated to Zig (ar_exit_instruction_evaluator.zig) to leverage Zig's `defer` mechanism for automatic cleanup, eliminating manual error handling cascades. The migration removed ~100+ lines of duplicated cleanup code by using `defer` statements that automatically execute when leaving scope, regardless of the return path.

## Purpose

This module extracts the agent destruction logic from the main exit instruction evaluator, following the single responsibility principle. It provides specialized handling for agent destruction with proper result storage.

## Key Components

### Instantiable Design

The module follows an instantiable design pattern with lifecycle management:

```c
// Create evaluator instance with dependencies (frame-based pattern)
ar_exit_instruction_evaluator_t* ar_exit_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

// Evaluate using frame-based execution
bool ar_exit_instruction_evaluator__evaluate(
    const ar_exit_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);

// Clean up instance
void ar_exit_instruction_evaluator__destroy(ar_exit_instruction_evaluator_t *own_evaluator);
```

### Legacy Interface

The module previously had a legacy parameter-based interface, but it has been fully migrated to the frame-based pattern. The legacy interface has been removed as part of the architectural modernization.

### Functionality

The module evaluates exit agent instructions of the form:
- `exit(agent_id)`
- `memory.result := exit(agent_id)`

Key features:
1. **Frame-Based Execution**: Uses ar_frame_t for memory, context, and message bundling
2. **Agent ID Evaluation**: Evaluates the agent ID expression to an integer
3. **Agent Destruction**: Exits the agent if it exists
4. **Result Assignment**: Stores true (1) if agent was terminated, false (0) otherwise
5. **Error Handling**: Returns false for invalid argument types
6. **Memory Access**: Gets memory from frame during evaluation

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator, memory, and log are borrowed references stored in the instance
- Agent ID evaluation creates temporary data that is properly cleaned up
- Result value is created and transferred to memory when assignment specified
- All temporary values properly exited
- The create function returns ownership to the caller
- The exit function takes ownership and frees all resources

### Zig Implementation Benefits

The Zig implementation provides significant advantages over the original C code:

1. **Automatic Cleanup with `defer`**: Resources are automatically freed when leaving scope, eliminating manual cleanup at each error return point
2. **Linear Code Flow**: No cascading error handling - each resource acquisition is followed by its `defer` cleanup
3. **Memory Safety**: Impossible to forget cleanup - the compiler ensures `defer` statements execute
4. **Reduced Code**: Eliminated ~100+ lines of duplicated cleanup logic
5. **Type Safety**: Uses `ar_allocator` module for type-safe memory operations
6. **Maintainability**: Adding new resources doesn't complicate existing error paths

Example of the pattern:
```zig
const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
defer ar_allocator.free(own_items);  // Automatically frees on any return

// ... use own_items without worrying about cleanup ...
```

## Dependencies

- `ar_log`: For centralized error reporting
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
4. Calls agency to exit the agent
5. Stores result if assignment specified

## Usage Examples

### Modern Instance-Based Approach (Recommended)

```c
// Create dependencies
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create exit agent evaluator instance
ar_exit_instruction_evaluator_t *evaluator = ar_exit_instruction_evaluator__create(
    log, expr_eval
);

// Parse exit instruction: memory.result := exit(42)
ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
    AR_INST__DESTROY, "exit", args, 1, "memory.result"
);

// Create frame for evaluation
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate using instance
bool success = ar_exit_instruction_evaluator__evaluate(evaluator, frame, ast);

// Clean up
ar_exit_instruction_evaluator__destroy(evaluator);
// Result stored in memory["result"]: 1 if exited, 0 if not found
```


## Testing

The module includes comprehensive tests covering:

### Instance-Based Interface Tests
- Create/exit lifecycle functions
- Instance-based evaluation using stored dependencies
- Legacy function backward compatibility

### Functional Tests
- Exit existing agent
- Exit with result assignment
- Exit nonexistent agent
- Invalid agent ID type
- Wrong number of arguments
- Memory leak verification

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.