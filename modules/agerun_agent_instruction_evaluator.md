# Agent Instruction Evaluator Module

## Overview

The agent instruction evaluator module is responsible for evaluating agent creation instructions in the AgeRun language. It handles creating new agents with specified methods and initial context.

## Purpose

This module extracts the agent instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for agent creation with proper context management.

## Key Components

### Public Interface

```c
bool ar__agent_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    const instruction_ast_t *ref_ast
);
```

### Functionality

The module evaluates agent instructions of the form:
- `agent(method_name, version, context)`
- `memory.agent_id := agent(method_name, version, context)`

Key features:
1. **Agent Creation**: Creates agents with specified method and context
2. **Method Validation**: Ensures the method exists before creating agent
3. **Context Handling**: Supports both memory and context references
4. **Wake Messages**: New agents receive `__wake__` message automatically
5. **Result Assignment**: Stores agent ID when assignment specified

### Context Optimization

The module includes an optimization for context handling:
- Direct memory/context references are passed without copying
- Complex expressions are evaluated and passed by value
- This avoids unnecessary data copying for common cases

### Memory Management

The module follows strict memory ownership rules:
- Method name and version evaluations are temporary
- Context is passed as reference (not owned by agent)
- Agent ID result is created when assignment specified
- All temporary values properly cleaned up

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions
- `agerun_expression_parser`: For parsing expression strings
- `agerun_expression_ast`: For expression AST nodes
- `agerun_instruction_ast`: For accessing instruction AST structure
- `agerun_agency`: For agent creation
- `agerun_methodology`: For method lookup
- `agerun_method`: For method references
- `agerun_data`: For data manipulation
- `agerun_string`: For string operations
- `agerun_heap`: For memory tracking

## Implementation Details

The module:
1. Evaluates method name and version to strings
2. Handles context evaluation with optimization for direct references
3. Validates method exists in methodology
4. Creates agent via agency
5. Stores agent ID if assignment specified

## Usage Example

```c
// Create evaluator with context
data_t *context = ar__data__create_map();
ar__data__set_map_string(context, "mode", "production");
expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, context);

// Parse agent instruction: memory.worker := agent("processor", "1.0.0", context)
instruction_ast_t *ast = ar__instruction_parser__parse_agent(parser);

// Evaluate the agent creation
bool success = ar__agent_instruction_evaluator__evaluate(
    expr_eval, memory, context, ast
);

// New agent created with ID stored in memory["worker"]
```

## Testing

The module includes comprehensive tests covering:
- Agent creation with context
- Agent ID result assignment
- Invalid method detection
- Various context types
- Invalid argument handling
- Memory leak verification

All tests pass with zero memory leaks.