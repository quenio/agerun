# Send Instruction Evaluator Module

## Overview

The send instruction evaluator module is responsible for evaluating send instructions in the AgeRun language. It handles sending messages to agents by their ID.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

**Implementation**: This module is implemented in Zig (`ar_send_instruction_evaluator.zig`) for improved error handling and memory safety while maintaining full C API compatibility.

## Purpose

This module extracts the send instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for agent messaging with proper memory management.

## Key Components

### Types

```c
typedef struct ar_send_instruction_evaluator_s ar_send_instruction_evaluator_t;
```

An opaque type representing a send instruction evaluator instance.

### Public Interface

```c
ar_send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
);
```
Creates a new send instruction evaluator with the provided log, expression evaluator, agency, and delegation instances. The evaluator borrows all dependencies and follows the frame-based execution pattern.

```c
void ar_send_instruction_evaluator__destroy(
    ar_send_instruction_evaluator_t *own_evaluator
);
```
Destroys a send instruction evaluator and frees all resources.

```c
bool ar_send_instruction_evaluator__evaluate(
    const ar_send_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates a send instruction using frame-based execution. The frame provides access to memory and other execution context.


### Functionality

The module evaluates send instructions of the form:
- `send(agent_id, message)`
- `memory.result := send(agent_id, message)`

Key features:
1. **Agent ID Evaluation**: Evaluates the agent ID expression to an integer
2. **Message Evaluation**: Evaluates the message expression to any data type
3. **Special Case ID 0**: Treats sends to agent 0 as no-op (returns true)
4. **Delegation Routing**: Routes negative IDs through the delegation subsystem and forwards the sender ID extracted from the frame context
5. **Result Assignment**: Stores the send result (boolean) when assignment is specified
6. **Ownership Coordination**: Transfers message ownership to the agency for positive IDs and borrows for delegation sends before destroying the local copy

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator, memory, and log are borrowed references stored in the instance
- Message ownership is transferred to `ar_agency__send_to_agent()`
- If send fails, the message is properly destroyed
- Result values are created and stored when assignment is specified
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_log`: For centralized error reporting
- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_agency`: For sending messages to agents (IDs >= 0)
- `ar_delegation`: For routing messages to delegates (IDs < 0) with sender metadata
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module evaluates both arguments:
1. Agent ID must evaluate to an integer
2. Message can be any data type
3. Uses helper functions for expression parsing and evaluation
4. Routes based on ID sign (>= 0 to agency, < 0 to delegation)
5. Handles result storage for assigned sends

## Usage Example

```c
// Create evaluator dependencies
ar_log_t *log = ar_log__create();
ar_data_t *memory = ar_data__create_map();
ar_data_t *context = ar_data__create_map();
ar_system_t *system = ar_system__create();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, context);

// Create send instruction evaluator
ar_agency_t *agency = ar_system__get_agency(system);
ar_delegation_t *delegation = ar_system__get_delegation(system);
ar_send_instruction_evaluator_t *send_eval = ar_send_instruction_evaluator__create(
    log, expr_eval, agency, delegation
);

// Parse send instruction: send(1, "Hello")
ar_instruction_ast_t *ast = ar_instruction_parser__parse_send(parser);

// Create a frame for evaluation (context stores sending agent for delegation)
ar_data__set_map_integer(context, "agent_id", agent_id);
ar_data_t *message = ar_data__create_string("test");
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate the send
bool success = ar_send_instruction_evaluator__evaluate(send_eval, frame, ast);

// Message "Hello" has been sent to agent 1

// Cleanup
ar_frame__destroy(frame);
ar_data__destroy(message);
ar_send_instruction_evaluator__destroy(send_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_system__destroy(system);
ar_data__destroy(context);
ar_data__destroy(memory);
ar_log__destroy(log);
```

## Testing

The module includes comprehensive tests covering:
- Sending to valid agents
- Sending to agent 0 (no-op case)
- Send with result assignment
- Various message types
- Invalid agent ID handling (errors reported through log)
- Memory leak verification

All tests pass with zero memory leaks. Errors are now reported through the centralized logging system rather than stored internally.