# Spawn Instruction Evaluator Module

## Overview

The spawn instruction evaluator module is responsible for evaluating spawn instructions in the AgeRun language. It handles creating new agents with specified methods and initial context.

**Implementation Note**: This module is implemented in Zig for improved memory safety and performance while maintaining full C compatibility.

## Purpose

This module extracts the agent instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for agent creation with proper context management.

## Key Components

### Instantiable Design

The module follows an instantiable design pattern with lifecycle management:

```c
// Spawn evaluator instance with dependencies (frame-based pattern)
ar_spawn_instruction_evaluator_t* ar_spawn_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

// Evaluate using frame-based execution
bool ar_spawn_instruction_evaluator__evaluate(
    const ar_spawn_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);

// Clean up instance
void ar_spawn_instruction_evaluator__destroy(ar_spawn_instruction_evaluator_t *own_evaluator);
```

### Functionality

The module evaluates spawn instructions of the form:
- `spawn(method_name, version, context)`
- `memory.agent_id := spawn(method_name, version, context)`

Key features:
1. **Frame-Based Execution**: Uses ar_frame_t for memory, context, and message bundling
2. **Agent Creation**: Spawns agents with specified method and context
3. **Method Validation**: Ensures the method exists before creating agent
4. **Context Handling**: Gets memory and context from execution frame
5. **Result Assignment**: Stores agent ID when assignment specified

### Frame-Based Architecture

The module follows the frame-based execution pattern:
- Memory, context, and message are bundled in an ar_frame_t
- Memory is retrieved from frame during evaluation
- Supports the unified frame-based evaluator architecture
- Consistent with other instruction evaluators (assignment, send, condition, etc.)

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator and log are borrowed references stored in the instance
- Method name and version use `ar_data__claim_or_copy` to ensure ownership
- Cleanup uses `ar_data__destroy_if_owned` for safe destruction
- Context is passed as a borrowed reference (never owned by agent)
- Agent ID result is created when assignment specified
- All temporary values properly cleaned up using Zig's defer mechanism
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_agency`: For agent creation
- `ar_methodology`: For method lookup
- `ar_method`: For method references
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_allocator`: For memory allocation (Zig implementation)

## Implementation Details

The module:
1. Evaluates method name and version expressions directly into `ar_data__claim_or_copy`
2. Uses `ar_data__destroy_if_owned` with defer for automatic cleanup
3. Context evaluation returns a borrowed reference (no ownership taken)
4. Validates method exists in methodology before agent creation
5. Creates agent via agency with borrowed context reference
6. Stores agent ID if assignment specified
7. Eliminates temporary variables by combining operations

## Usage Examples

### Modern Instance-Based Approach (Recommended)

```c
// Create dependencies
ar_log_t *log = ar_log__create();
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create spawn instruction evaluator instance
ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
    log, expr_eval
);

// Create frame for execution
ar_frame_t *frame = ar_frame__create(memory, NULL, NULL);

// Parse spawn instruction: memory.worker := spawn("processor", "1.0.0", context)
ar_instruction_ast_t *ast = ar_instruction_parser__parse_spawn(parser);

// Evaluate using instance (note: evaluator is const)
bool success = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);

// Clean up
ar_spawn_instruction_evaluator__destroy(evaluator);
ar_frame__destroy(frame);
// New agent spawned with ID stored in memory["worker"]
```


## Testing

The module includes comprehensive tests covering:

### Interface Tests
- Create/destroy lifecycle functions
- Evaluation with frame-based execution
- Const evaluator parameter verification

### Functional Tests  
- Agent creation with context
- Agent ID result assignment
- Invalid method detection
- Various context types
- Invalid argument handling
- Memory leak verification

All tests pass with zero memory leaks. The test suite follows TDD methodology with Red-Green-Refactor cycles. Errors are now reported through the centralized logging system rather than stored internally.