# Send Instruction Evaluator Module

## Overview

The send instruction evaluator module is responsible for evaluating send instructions in the AgeRun language. It handles sending messages to agents by their ID.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

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
    ar_expression_evaluator_t *ref_expr_evaluator
);
```
Creates a new send instruction evaluator with the provided log and expression evaluator. The evaluator follows the frame-based execution pattern and does not store memory references.

```c
void ar_send_instruction_evaluator__destroy(
    ar_send_instruction_evaluator_t *own_evaluator
);
```
Destroys a send instruction evaluator and frees all resources.

```c
bool ar_send_instruction_evaluator__evaluate(
    ar_send_instruction_evaluator_t *mut_evaluator,
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
4. **Result Assignment**: Stores the send result (boolean) when assignment is specified
5. **Ownership Transfer**: Transfers message ownership to the agency

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
- `ar_agency`: For sending messages to agents
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module evaluates both arguments:
1. Agent ID must evaluate to an integer
2. Message can be any data type
3. Uses helper functions for expression parsing and evaluation
4. Handles result storage for assigned sends

## Usage Example

```c
// Create memory and expression evaluator
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

// Create send instruction evaluator
ar_send_instruction_evaluator_t *send_eval = ar_send_instruction_evaluator__create(
    log, expr_eval, memory
);

// Parse send instruction: send(1, "Hello")
ar_instruction_ast_t *ast = ar_instruction_parser__parse_send(parser);

// Evaluate the send
bool success = ar_send_instruction_evaluator__evaluate(send_eval, ast);

// Message "Hello" has been sent to agent 1

// Cleanup
ar_send_instruction_evaluator__destroy(send_eval);
ar_expression_evaluator__destroy(expr_eval);
ar_data__destroy(memory);
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