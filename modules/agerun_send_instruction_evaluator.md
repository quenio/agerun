# Send Instruction Evaluator Module

## Overview

The send instruction evaluator module is responsible for evaluating send instructions in the AgeRun language. It handles sending messages to agents by their ID.

## Purpose

This module extracts the send instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for agent messaging with proper memory management.

## Key Components

### Public Interface

```c
bool ar_send_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);
```

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
- Message ownership is transferred to `ar__agency__send_to_agent()`
- If send fails, the message is properly destroyed
- Result values are created and stored when assignment is specified

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions
- `agerun_expression_parser`: For parsing expression strings
- `agerun_expression_ast`: For expression AST nodes
- `agerun_instruction_ast`: For accessing instruction AST structure
- `agerun_agency`: For sending messages to agents
- `agerun_data`: For data manipulation
- `agerun_string`: For string operations
- `agerun_heap`: For memory tracking

## Implementation Details

The module evaluates both arguments:
1. Agent ID must evaluate to an integer
2. Message can be any data type
3. Uses helper functions for expression parsing and evaluation
4. Handles result storage for assigned sends

## Usage Example

```c
// Create evaluator
expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Parse send instruction: send(1, "Hello")
instruction_ast_t *ast = ar__instruction_parser__parse_send(parser);

// Evaluate the send
bool success = ar_send_instruction_evaluator__evaluate(
    expr_eval, memory, ast
);

// Message "Hello" has been sent to agent 1
```

## Testing

The module includes comprehensive tests covering:
- Sending to valid agents
- Sending to agent 0 (no-op case)
- Send with result assignment
- Various message types
- Invalid agent ID handling
- Memory leak verification

All tests pass with zero memory leaks.