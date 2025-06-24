# Instruction Evaluator Module

The instruction evaluator module (`ar_instruction_evaluator`) serves as the central coordinator for executing parsed instruction AST nodes within the AgeRun runtime environment. It manages a collection of specialized instruction evaluators, each responsible for a specific instruction type.

## Purpose

The instruction evaluator acts as a facade that coordinates specialized instruction evaluators, providing a unified interface for instruction execution. It:

- Manages instances of all specialized instruction evaluators
- Delegates evaluation requests to the appropriate specialized evaluator
- Provides a single entry point for instruction evaluation
- Maintains shared dependencies (expression evaluator, memory, context)
- Ensures consistent error handling across all instruction types

## Architecture

### Specialized Evaluator Pattern

The module follows a **composition pattern** where it creates and manages instances of specialized evaluators:

- `assignment_instruction_evaluator`: Handles memory assignments
- `send_instruction_evaluator`: Handles agent messaging
- `condition_instruction_evaluator`: Handles conditional expressions
- `parse_instruction_evaluator`: Handles template parsing
- `build_instruction_evaluator`: Handles template building
- `method_instruction_evaluator`: Handles method creation
- `agent_instruction_evaluator`: Handles agent creation
- `destroy_agent_instruction_evaluator`: Handles agent destruction
- `destroy_method_instruction_evaluator`: Handles method destruction

### Design Principles

- **Single Responsibility**: Each specialized evaluator handles one instruction type
- **Composition over Inheritance**: Uses dependency injection for specialized evaluators
- **Memory Safety**: Strict ownership semantics with proper cleanup
- **Extensibility**: Easy to add new instruction types by creating new specialized evaluators
- **Separation of Concerns**: Clear boundaries between different instruction types

## Dependencies

- All 9 specialized instruction evaluator modules
- `ar_expression_evaluator`: For evaluating expressions within instructions
- `ar_instruction_ast`: For accessing parsed instruction structures
- `ar_data`: For data manipulation and storage
- `ar_heap`: For memory tracking

## API Reference

### Types

```c
typedef struct instruction_evaluator_s ar_instruction_evaluator_t;
```

An opaque type representing an instruction evaluator instance.

### Lifecycle Functions

#### ar_instruction_evaluator__create

```c
ar_instruction_evaluator_t* ar_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
);
```

Creates a new instruction evaluator instance and initializes all specialized evaluators.

**Parameters:**
- `ref_expr_evaluator`: Expression evaluator to use (required, borrowed reference)
- `mut_memory`: Memory map for storing values (required, mutable reference)
- `ref_context`: Optional context map (can be NULL, borrowed reference)
- `ref_message`: Optional message data (can be NULL, borrowed reference)

**Returns:** New evaluator instance or NULL on failure

**Ownership:** Caller owns the returned evaluator and must destroy it

**Behavior:**
- Creates instances of all 9 specialized instruction evaluators
- Passes shared dependencies to each specialized evaluator
- Returns NULL if any specialized evaluator creation fails

#### ar_instruction_evaluator__destroy

```c
void ar_instruction_evaluator__destroy(ar_instruction_evaluator_t *own_evaluator);
```

Destroys an instruction evaluator instance and all its specialized evaluators.

**Parameters:**
- `own_evaluator`: The evaluator to destroy (owned, can be NULL)

**Behavior:**
- Destroys all specialized evaluator instances
- Frees the main evaluator structure
- Safe to call with NULL pointer

### Delegation Functions

Each delegation function follows the same pattern:
- Takes the evaluator and an instruction AST node
- Delegates to the appropriate specialized evaluator
- Returns `true` on successful evaluation, `false` on failure
- All memory management and error handling is done by the specialized evaluator

#### ar_instruction_evaluator__evaluate_assignment

```c
bool ar_instruction_evaluator__evaluate_assignment(
    ar_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);
```

Delegates assignment instruction evaluation to the assignment_instruction_evaluator.

**Behavior:**
- Calls `ar_assignment_instruction_evaluator__evaluate()` with the stored instance
- The specialized evaluator handles all assignment logic including path validation and value storage

The instruction evaluator provides delegation functions for each instruction type. Each function simply delegates to its corresponding specialized evaluator:

#### All Delegation Functions

```c
// Delegates to assignment_instruction_evaluator
bool ar_instruction_evaluator__evaluate_assignment(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Delegates to send_instruction_evaluator
bool ar_instruction_evaluator__evaluate_send(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Delegates to condition_instruction_evaluator
bool ar_instruction_evaluator__evaluate_if(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Delegates to parse_instruction_evaluator
bool ar_instruction_evaluator__evaluate_parse(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Delegates to build_instruction_evaluator
bool ar_instruction_evaluator__evaluate_build(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Delegates to method_instruction_evaluator
bool ar_instruction_evaluator__evaluate_method(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Delegates to agent_instruction_evaluator
bool ar_instruction_evaluator__evaluate_agent(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);

// Dispatches to destroy_agent_instruction_evaluator or destroy_method_instruction_evaluator
bool ar_instruction_evaluator__evaluate_destroy(
    ar_instruction_evaluator_t *mut_evaluator, const ar_instruction_ast_t *ref_ast);
```

**Common Behavior:**
- All functions delegate to their corresponding specialized evaluator instance
- Return `true` on successful evaluation, `false` on failure
- The specialized evaluators handle all logic, memory management, and error handling
- For detailed behavior of each instruction type, see the individual specialized evaluator documentation

## Usage Examples

### Basic Assignment

```c
// Create evaluator with dependencies
ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
    expr_eval, memory, NULL, NULL
);

// Create assignment AST: memory.count := 10
ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment(
    "memory.count", "10"
);

// Evaluate (delegates to assignment_instruction_evaluator)
bool success = ar_instruction_evaluator__evaluate_assignment(evaluator, ast);

// Clean up
ar_instruction_ast__destroy(ast);
ar_instruction_evaluator__destroy(evaluator);
```

### Agent Messaging

```c
// Create send AST: send(1, "Hello")
const char *args[] = {"1", "\"Hello\""};
ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
    AR_INST__SEND, "send", args, 2, NULL
);

// Evaluate (delegates to send_instruction_evaluator)
bool success = ar_instruction_evaluator__evaluate_send(evaluator, ast);
```

### Coordination Pattern

The instruction evaluator demonstrates the **facade pattern** by providing a unified interface that coordinates multiple specialized evaluators:

```c
// Single evaluator manages all specialized evaluators
ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
    expr_eval, memory, context, message
);

// Each instruction type is handled by its specialized evaluator
ar_instruction_evaluator__evaluate_assignment(evaluator, assignment_ast);
ar_instruction_evaluator__evaluate_send(evaluator, send_ast);
ar_instruction_evaluator__evaluate_if(evaluator, condition_ast);
// ... etc for all instruction types

// Single destroy call cleans up all specialized evaluators
ar_instruction_evaluator__destroy(evaluator);
```

## Memory Management

### Ownership Rules

1. **Evaluator Composition**: The instruction evaluator owns all specialized evaluator instances
2. **Dependency Management**: Shared dependencies (expression evaluator, memory, context) are passed to all specialized evaluators
3. **Delegation Pattern**: All memory management is handled by the specialized evaluators
4. **Clean Destruction**: Destroying the instruction evaluator properly destroys all specialized evaluators

### Architecture Benefits

- **Single Point of Control**: One evaluator manages all specialized instances
- **Consistent Dependencies**: All specialized evaluators share the same dependencies
- **Proper Lifecycle**: Creation and destruction are coordinated across all evaluators
- **Memory Safety**: Each specialized evaluator handles its own memory management

## Error Handling

Error handling is delegated to the specialized evaluators:

- Each specialized evaluator provides its own error checking and validation
- The instruction evaluator simply returns the result from the specialized evaluator
- No additional error handling logic is needed at the coordination level
- Consistent error reporting across all instruction types

## Performance Considerations

- **Initialization Cost**: Creating an instruction evaluator creates 9 specialized evaluator instances
- **Runtime Efficiency**: Delegation calls have minimal overhead
- **Memory Locality**: All evaluators are created together and can benefit from cache locality
- **Reusability**: Single instruction evaluator can handle all instruction types efficiently

## Testing

The module has a focused test suite that verifies the coordination pattern:

- `ar_instruction_evaluator_tests.c`: Tests creation/destruction and delegation functions
- **Specialized evaluator testing**: Each specialized evaluator has its own comprehensive test suite
- **Integration focus**: Tests verify that delegation works correctly
- **Memory safety**: All tests pass with zero memory leaks
- **Architecture validation**: Tests confirm that specialized evaluators are properly created and managed

### Testing Philosophy

The instruction evaluator tests focus on:

1. **Composition**: Verifying all specialized evaluators are created
2. **Delegation**: Ensuring calls are properly forwarded to specialized evaluators  
3. **Lifecycle**: Testing create/destroy coordination
4. **Integration**: Confirming the facade pattern works correctly

Detailed functionality testing is handled by each specialized evaluator's test suite, following the single responsibility principle.

## Future Enhancements

- Support optional parameters (e.g., agent creation without context)
- Add support for memory references in send() without copying
- Implement expression result caching for performance
- Add detailed error messages with position information
- Support for custom evaluation contexts