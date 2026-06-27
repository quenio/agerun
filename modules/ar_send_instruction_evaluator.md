# Send Instruction Evaluator Module

## Overview

The send instruction evaluator module is responsible for evaluating send instructions in the AgeRun language. It implements **ID-based message routing**, directing messages to either agents (positive IDs) or delegates (negative IDs) based on the target ID sign.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

**Implementation**: This module is implemented in Zig (`ar_send_instruction_evaluator.zig`) for improved error handling and memory safety while maintaining full C API compatibility.

## Purpose

This module extracts the send instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for message routing with proper memory management and ID-based destination resolution.

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

Creates a new send instruction evaluator with the provided log, expression evaluator, agency, and delegation instances. The evaluator implements ID-based routing to direct messages to the appropriate destination.

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
- `send(recipient, message)`
- `memory.result := send(recipient, message)`

Key features:
1. **Recipient Evaluation**: Evaluates the recipient expression; only nonzero INTEGER values route
2. **Message Evaluation**: Evaluates the message expression to any data type
3. **ID-Based Routing**: Routes messages based on ID sign:
   - **ID > 0**: Routes to agency for agent delivery
   - **ID < 0**: Routes to delegation for delegate delivery
   - **ID == 0 or non-INTEGER recipient**: No-delivery sink from the central
     [SPEC.md sentinel contract](../SPEC.md#integer-0-sentinel-semantics); no message is delivered,
     and assigned calls store integer `0` because no message was sent
4. **Result Assignment**: Stores integer `1` or `0` through `ar_result_binding` when assignment is specified
5. **Ownership Transfer**: Transfers message ownership to appropriate destination (agency or delegation)
6. **Nested Payloads**: Deep-copies borrowed nested list/map messages before delivery

## Message Routing Architecture

The send instruction evaluator implements **ID-based message routing** following the pattern documented in [id-based-message-routing-pattern.md](../kb/id-based-message-routing-pattern.md):

| Target ID | Destination | Function Called |
|-----------|-------------|-----------------|
| `0` | No-delivery sink | N/A - no delivery status |
| `> 0` | Agent via agency | `ar_agency__send_to_agent()` |
| `< 0` | Delegate via delegation | `ar_delegation__send_to_delegate()` |
| non-INTEGER | No-delivery sink | N/A - no delivery status |

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Log, expression evaluator, agency, and delegation are borrowed references
- Message ownership is transferred to either agency or delegation depending on ID
- Borrowed message values are claimed or deep-copied before delivery, preserving nested payloads
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
- `ar_agency`: For sending messages to agents (positive IDs)
- `ar_delegation`: For sending messages to delegates (negative IDs)
- `ar_result_binding`: For assigned effectful result-path validation and owned-result storage
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module evaluates both arguments:
1. Recipient values route only when they evaluate to nonzero integers
2. Message can be any data type
3. Routes nonzero integers based on ID sign to the appropriate destination
4. Validates assigned result targets before sending so protected `memory.self` writes are rejected
5. Handles assigned send result storage through `ar_result_binding`
6. Properly manages ownership transfer based on destination

## Usage Examples

### Sending to an Agent

```c
// Create evaluator dependencies
ar_log_t *log = ar_log__create();
ar_data_t *memory = ar_data__create_map();
ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory);
ar_agency_t *agency = ar_agency__create(log);
ar_delegation_t *delegation = ar_delegation__create(log);

// Create send instruction evaluator
ar_send_instruction_evaluator_t *send_eval = ar_send_instruction_evaluator__create(
    log, expr_eval, agency, delegation
);

// Parse send instruction: send(1, "Hello")
ar_instruction_ast_t *ast = ar_instruction_parser__parse_send(parser);

// Create a frame for evaluation
ar_frame_t *frame = ar_frame__create(memory);

// Evaluate the send
bool success = ar_send_instruction_evaluator__evaluate(send_eval, frame, ast);

// Message "Hello" has been sent to agent 1
```

### Sending to a Delegate

```c
// Same setup as above...

// Send to delegate ID -1: send(-1, {"action": "read"})
ar_instruction_ast_t *ast = ar_instruction_parser__parse_send(parser);
bool success = ar_send_instruction_evaluator__evaluate(send_eval, frame, ast);

// Message has been sent to delegate -1 via delegation system
```

### Sending with Result Assignment

```c
// Send with result: memory.result := send(1, "test")
ar_instruction_ast_t *ast = ar_instruction_parser__parse_send(parser);
bool success = ar_send_instruction_evaluator__evaluate(send_eval, frame, ast);

// Result status (integer 1/0) is now stored in memory.result
```

## Error Handling

The send instruction evaluator handles errors gracefully:

| Error Case | Behavior | Evaluator Status | Assigned Result |
|------------|----------|--------------|------------------|
| Non-existent agent | Agent not found in agency; message destroyed | Continues if assigned result stores | Integer `0` |
| Non-existent delegate | Delegate not found in delegation; message destroyed | Continues if assigned result stores | Integer `0` |
| Recipient expression evaluation fails | Message expression is not evaluated | Fails | No result stored |
| Non-INTEGER recipient | No message is delivered | Continues | Integer `0` |
| Invalid message expression | Expression evaluation fails | Fails | No result stored |
| INTEGER `0` recipient | No message is delivered | Continues | Integer `0` |

All error paths ensure proper memory cleanup - messages are destroyed when delivery fails to prevent memory leaks.

## Testing

The module includes comprehensive tests covering:
- Sending to valid agents (positive IDs)
- Sending to valid delegates (negative IDs)
- Sending to non-routable recipients, including integer `0` and non-INTEGER values
- Send with result assignment
- Various message types (integers, strings, maps, lists)
- Invalid agent ID handling (stores integer `0` status when assigned)
- Delegate routing verification (message actually queued)
- Memory leak verification
- Message ownership transfer validation

All tests pass with zero memory leaks. Errors are reported through the centralized logging system.
