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

### Legacy Interface (Backward Compatibility)

```c
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
5. **Wake Messages**: New agents receive `__wake__` message automatically
6. **Result Assignment**: Stores agent ID when assignment specified

### Frame-Based Architecture

The module follows the frame-based execution pattern:
- Memory, context, and message are bundled in an ar_frame_t
- Memory is retrieved from frame during evaluation
- Supports the unified frame-based evaluator architecture
- Consistent with other instruction evaluators (assignment, send, condition, etc.)

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator, memory, and log are borrowed references stored in the instance
- Method name and version evaluations are temporary
- Context is passed as reference (not owned by agent)
- Agent ID result is spawnd when assignment specified
- All temporary values properly cleaned up
- The spawn function returns ownership to the caller
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
1. Evaluates method name and version to strings
2. Handles context evaluation with optimization for direct references
3. Validates method exists in methodology
4. Spawns agent via agency
5. Stores agent ID if assignment specified

## Usage Examples

### Modern Instance-Based Approach (Recommended)

```c
// Spawn dependencies
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Spawn agent evaluator instance
ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
    log, expr_eval, memory
);

// Parse spawn instruction: memory.worker := spawn("processor", "1.0.0", context)
ar_instruction_ast_t *ast = ar_instruction_parser__parse_agent(parser);

// Evaluate using instance
bool success = ar_spawn_instruction_evaluator__evaluate(evaluator, ast);

// Clean up
ar_spawn_instruction_evaluator__destroy(evaluator);
// New agent spawnd with ID stored in memory["worker"]
```

### Legacy Approach (Backward Compatibility)

```c
```

## Testing

The module includes comprehensive tests covering:

### Instance-Based Interface Tests
- Spawn/destroy lifecycle functions
- Instance-based evaluation using stored dependencies
- Legacy function backward compatibility

### Functional Tests  
- Agent creation with context
- Agent ID result assignment
- Invalid method detection
- Various context types
- Invalid argument handling
- Memory leak verification

All tests pass with zero memory leaks. The test suite follows TDD methodology with Red-Green-Refactor cycles. Errors are now reported through the centralized logging system rather than stored internally.