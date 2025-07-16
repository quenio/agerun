# Instruction Evaluator Module

The instruction evaluator module (`ar_instruction_evaluator`) serves as the central coordinator for executing parsed instruction AST nodes within the AgeRun runtime environment. It manages a collection of specialized instruction evaluators, each responsible for a specific instruction type.

## Purpose

The instruction evaluator acts as a facade that coordinates specialized instruction evaluators, providing a unified interface for instruction execution. It:

- Manages instances of all specialized instruction evaluators
- Delegates evaluation requests to the appropriate specialized evaluator
- Provides a single entry point for instruction evaluation
- Maintains shared dependencies (expression evaluator, log)
- Creates sub-evaluators on-demand with memory from execution frame
- Ensures consistent error reporting through centralized logging

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
- `ar_expression_evaluator`: Created and owned internally for evaluating expressions
- `ar_instruction_ast`: For accessing parsed instruction structures
- `ar_data`: For data manipulation and storage
- `ar_heap`: For memory tracking
- `ar_log`: For centralized error reporting

## API Reference

### Types

```c
typedef struct ar_instruction_evaluator_s ar_instruction_evaluator_t;
```

An opaque type representing an instruction evaluator instance.

### Lifecycle Functions

#### ar_instruction_evaluator__create

```c
ar_instruction_evaluator_t* ar_instruction_evaluator__create(
    ar_log_t *ref_log
);
```

Creates a new instruction evaluator instance.

**Parameters:**
- `ref_log`: Log instance for error reporting (required, borrowed reference)

**Returns:** New evaluator instance or NULL on failure

**Ownership:** Caller owns the returned evaluator and must destroy it

**Behavior:**
- Creates and owns an expression evaluator internally
- Creates all specialized instruction evaluators immediately
- All sub-evaluators share the same log and expression evaluator
- Returns NULL if log or expression evaluator is NULL

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

### Evaluation Function

#### ar_instruction_evaluator__evaluate

```c
bool ar_instruction_evaluator__evaluate(
    ar_instruction_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);
```

Evaluates any instruction AST node by dispatching to the appropriate specialized evaluator.

**Parameters:**
- `mut_evaluator`: The evaluator instance (mutable reference)
- `ref_frame`: The execution frame containing memory, context, and message (borrowed reference)
- `ref_ast`: The instruction AST node to evaluate (borrowed reference)

**Returns:** `true` on successful evaluation, `false` on failure

**Behavior:**
- Determines the instruction type from the AST node
- Creates the appropriate specialized evaluator on-demand if not already created
- Passes the frame to specialized evaluators (for assignment) or extracts memory/context as needed
- Delegates evaluation to the appropriate specialized evaluator
- Returns the result from the specialized evaluator

**Frame-Based Evaluation:**
- The assignment evaluator receives the frame directly (frame-based pattern)
- Other evaluators still receive memory/context extracted from the frame (legacy pattern)
- Sub-evaluators are created lazily using memory from the frame

## Usage Examples

### Basic Assignment

```c
// Create evaluator with just a log
ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log);

// Create assignment AST: memory.count := 10
ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment(
    "memory.count", "10"
);

// Create frame with memory, context, and message
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate using unified interface
bool success = ar_instruction_evaluator__evaluate(evaluator, frame, ast);

// Clean up
ar_frame__destroy(frame);
ar_instruction_ast__destroy(ast);
ar_instruction_evaluator__destroy(evaluator);
```

### Agent Messaging

```c
// Create send AST: send(1, "Hello")
const char *args[] = {"1", "\"Hello\""};
ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
    AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
);

// Create frame
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate using unified interface
bool success = ar_instruction_evaluator__evaluate(evaluator, frame, ast);

// Clean up
ar_frame__destroy(frame);
```

### Coordination Pattern

The instruction evaluator demonstrates the **facade pattern** by providing a unified interface that coordinates multiple specialized evaluators:

```c
// Create evaluator - it manages its own expression evaluator
ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log);

// Create frame for evaluation
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Unified interface handles all instruction types
ar_instruction_evaluator__evaluate(evaluator, frame, assignment_ast);
ar_instruction_evaluator__evaluate(evaluator, frame, send_ast);
ar_instruction_evaluator__evaluate(evaluator, frame, condition_ast);
// ... etc for all instruction types

// Clean up
ar_frame__destroy(frame);
ar_instruction_evaluator__destroy(evaluator);
```

## Memory Management

### Ownership Rules

1. **Lazy Initialization**: Specialized evaluators are created on-demand when first needed
2. **Frame-Based Resources**: Memory, context, and message come from the execution frame
3. **Delegation Pattern**: All memory management is handled by the specialized evaluators
4. **Clean Destruction**: Destroying the instruction evaluator properly destroys all created specialized evaluators

### Architecture Benefits

- **Single Point of Control**: One evaluator manages all specialized instances
- **Consistent Dependencies**: All specialized evaluators share the same dependencies
- **Proper Lifecycle**: Creation and destruction are coordinated across all evaluators
- **Memory Safety**: Each specialized evaluator handles its own memory management

## Error Handling

Error handling uses the centralized logging system:

- All errors are reported through the ar_log instance passed at creation
- Each specialized evaluator reports errors to the shared log
- The instruction evaluator simply returns the result from the specialized evaluator
- No additional error handling logic is needed at the coordination level
- Consistent error reporting across all instruction types through centralized logging

## Performance Considerations

- **Lazy Initialization**: Specialized evaluators are created only when needed, reducing startup cost
- **Runtime Efficiency**: First use of each instruction type has creation overhead, subsequent uses are fast
- **Memory Usage**: Only creates evaluators for instruction types actually used
- **Reusability**: Single instruction evaluator can handle all instruction types efficiently

## Testing

The module has a focused test suite that verifies the coordination pattern:

- `instruction_evaluator_tests.c`: Tests creation/destruction and delegation functions
- **Specialized evaluator testing**: Each specialized evaluator has its own comprehensive test suite
- **Integration focus**: Tests verify that delegation works correctly
- **Memory safety**: All tests pass with zero memory leaks
- **Architecture validation**: Tests confirm that specialized evaluators are properly created and managed

### Testing Philosophy

The instruction evaluator tests focus on:

1. **Lazy Creation**: Verifying specialized evaluators are created on-demand
2. **Unified Interface**: Ensuring the single evaluate method dispatches correctly
3. **Frame Integration**: Testing that memory is extracted from frames properly
4. **Lifecycle**: Testing create/destroy coordination with lazy initialization

Detailed functionality testing is handled by each specialized evaluator's test suite, following the single responsibility principle.

## Future Enhancements

- Support optional parameters (e.g., agent creation without context)
- Add support for memory references in send() without copying
- Implement expression result caching for performance
- Add detailed error messages with position information
- Support for custom evaluation contexts