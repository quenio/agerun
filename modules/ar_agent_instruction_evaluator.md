# Agent Instruction Evaluator Module

## Overview

The agent instruction evaluator module is responsible for evaluating agent creation instructions in the AgeRun language. It handles creating new agents with specified methods and initial context.

## Purpose

This module extracts the agent instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for agent creation with proper context management.

## Key Components

### Instantiable Design

The module follows an instantiable design pattern with lifecycle management:

```c
// Create evaluator instance with dependencies
agent_instruction_evaluator_t* ar_agent_instruction_evaluator__create(
    ar_expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
);

// Evaluate using stored dependencies
bool ar_agent_instruction_evaluator__evaluate(
    const agent_instruction_evaluator_t *ref_evaluator,
    data_t *ref_context,
    const ar_instruction_ast_t *ref_ast
);

// Clean up instance
void ar_agent_instruction_evaluator__destroy(agent_instruction_evaluator_t *own_evaluator);
```

### Legacy Interface (Backward Compatibility)

```c
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

- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_agency`: For agent creation
- `ar_methodology`: For method lookup
- `ar_method`: For method references
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module:
1. Evaluates method name and version to strings
2. Handles context evaluation with optimization for direct references
3. Validates method exists in methodology
4. Creates agent via agency
5. Stores agent ID if assignment specified

## Usage Examples

### Modern Instance-Based Approach (Recommended)

```c
// Create dependencies
data_t *memory = ar__data__create_map();
ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Create agent evaluator instance
agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(
    expr_eval, memory
);

// Parse agent instruction: memory.worker := agent("processor", "1.0.0", context)
ar_instruction_ast_t *ast = ar_instruction_parser__parse_agent(parser);

// Evaluate using instance
bool success = ar_agent_instruction_evaluator__evaluate(evaluator, context, ast);

// Clean up
ar_agent_instruction_evaluator__destroy(evaluator);
// New agent created with ID stored in memory["worker"]
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
- Agent creation with context
- Agent ID result assignment
- Invalid method detection
- Various context types
- Invalid argument handling
- Memory leak verification

All tests pass with zero memory leaks (511 allocations, 0 active). The test suite follows TDD methodology with Red-Green-Refactor cycles.