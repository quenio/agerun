# Method Evaluator Module

The method evaluator module (`ar_method_evaluator`) provides functionality to evaluate parsed method ASTs, executing all instructions within a method in sequence. It acts as the bridge between the method parser and the interpreter, enabling "parse once, evaluate many times" semantics.

## Purpose

The method evaluator serves as the execution engine for parsed methods within the AgeRun runtime. It:

- Evaluates pre-parsed method ASTs without reparsing
- Executes instructions sequentially using frames for context
- Manages stateless expression and instruction evaluators internally
- Provides a reusable evaluator that can execute the same method with different frames
- Enables efficient method execution in the interpreter

## Architecture

### Key Design Decisions

1. **Parse Once, Evaluate Many**: Methods are parsed into ASTs once, then the evaluator can execute them repeatedly with different execution contexts (frames)

2. **Stateless Evaluation**: Uses stateless expression and instruction evaluators internally, with frames providing the execution context

3. **Sequential Execution**: Instructions are executed in order, with execution stopping on the first error

4. **Evaluator Reuse**: A single evaluator instance can be reused across multiple method executions with different frames

## Dependencies

- `ar_method`: For accessing the method's parsed AST
- `ar_method_ast`: For iterating through instruction ASTs
- `ar_instruction_ast`: For instruction AST structures
- `ar_expression_evaluator`: For creating stateless expression evaluator
- `ar_instruction_evaluator`: For creating stateless instruction evaluator
- `ar_frame`: For execution context (memory, context, message)
- `ar_heap`: For memory tracking
- `ar_io`: For error reporting

## API Reference

### Types

```c
typedef struct ar_method_evaluator_s ar_method_evaluator_t;
```

An opaque type representing a method evaluator instance.

### Lifecycle Functions

#### ar_method_evaluator__create

```c
ar_method_evaluator_t* ar_method_evaluator__create(
    const method_t *ref_method
);
```

Creates a new method evaluator for a specific method.

**Parameters:**
- `ref_method`: The method containing parsed ASTs to evaluate (borrowed reference)

**Returns:** A new evaluator instance or NULL on failure

**Ownership:** Returns an owned value that caller must destroy

**Behavior:**
- Validates that the method has a parsed AST
- Creates stateless expression and instruction evaluators internally
- Stores a reference to the method (does not take ownership)

#### ar_method_evaluator__destroy

```c
void ar_method_evaluator__destroy(ar_method_evaluator_t *own_evaluator);
```

Destroys a method evaluator and its internal evaluators.

**Parameters:**
- `own_evaluator`: The evaluator to destroy (owned, can be NULL)

**Behavior:**
- Destroys the internal expression and instruction evaluators
- Frees all allocated memory
- Safe to call with NULL

### Evaluation Functions

#### ar_method_evaluator__evaluate

```c
bool ar_method_evaluator__evaluate(
    ar_method_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame
);
```

Evaluates a method using the provided frame.

**Parameters:**
- `mut_evaluator`: The evaluator instance (mutable reference)
- `ref_frame`: The frame containing memory, context, and message (borrowed reference)

**Returns:** true if all instructions executed successfully, false on error

**Behavior:**
- Retrieves the method's parsed AST
- Executes each instruction in sequence using the frame
- Stops execution on the first error
- Reports detailed error messages with instruction numbers

## Usage Examples

### Basic Method Evaluation

```c
// Create a method with instructions
method_t *method = ar__method__create(
    "calculate",
    "memory.x := 10\n"
    "memory.y := 20\n"
    "memory.sum := memory.x + memory.y",
    "1.0.0"
);

// Create an evaluator for the method
ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);

// Create a frame with execution context
data_t *memory = ar__data__create_map();
data_t *context = ar__data__create_map();
data_t *message = ar__data__create_string("start");
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate the method
bool success = ar_method_evaluator__evaluate(evaluator, frame);
if (success) {
    // Access results from memory
    data_t *sum = ar__data__get_map_data(memory, "sum");
    printf("Sum: %lld\n", ar__data__get_integer(sum));
}

// Clean up
ar_frame__destroy(frame);
ar__data__destroy(memory);
ar__data__destroy(context);
ar__data__destroy(message);
ar_method_evaluator__destroy(evaluator);
ar__method__destroy(method);
```

### Evaluator Reuse

```c
// Create evaluator once
ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);

// Use with multiple frames
for (int i = 0; i < 5; i++) {
    data_t *memory = ar__data__create_map();
    ar__data__set_map_data(memory, "input", ar__data__create_integer(i));
    
    data_t *context = ar__data__create_map();
    data_t *message = ar__data__create_integer(i);
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    
    // Evaluate with different data each time
    bool success = ar_method_evaluator__evaluate(evaluator, frame);
    
    // Process results...
    
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
}

// Destroy evaluator once
ar_method_evaluator__destroy(evaluator);
```

### Error Handling

```c
// Method with potential error
method_t *method = ar__method__create(
    "risky",
    "memory.text := \"hello\"\n"
    "memory.bad := memory.text + 5",  // Type error
    "1.0.0"
);

ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
// ... create frame ...

bool success = ar_method_evaluator__evaluate(evaluator, frame);
if (!success) {
    // Error message will indicate which instruction failed
    // e.g., "Failed to evaluate instruction 2"
}
```

## Memory Management

### Ownership Rules

1. **Method Reference**: The evaluator keeps a borrowed reference to the method - it does not own it
2. **Internal Evaluators**: The evaluator owns its expression and instruction evaluators
3. **Frame Reference**: The frame is borrowed during evaluation - the evaluator does not modify or own it
4. **No Result Ownership**: Results are stored in the frame's memory - the caller manages them

### Memory Safety

- All allocations use heap tracking macros
- Proper cleanup in error paths
- No circular dependencies
- Safe with NULL parameters

## Error Handling

The module provides detailed error reporting:

- **NULL Parameters**: Reports specific parameter errors
- **Missing AST**: Reports if method has no parsed AST
- **Instruction Errors**: Reports which instruction number failed
- **Evaluation Errors**: Propagates errors from instruction evaluator

Error messages include the function name and specific details about what failed.

## Performance Considerations

- **Parse Once**: Methods are parsed once when created, not during evaluation
- **Evaluator Reuse**: Single evaluator can handle multiple executions
- **Stateless Design**: Minimal overhead from frame-based evaluation
- **Early Exit**: Stops on first error to avoid unnecessary work

## Testing

The module has comprehensive test coverage:

- **Create/Destroy**: Basic lifecycle testing
- **Simple Assignment**: Single instruction evaluation
- **Multiple Instructions**: Sequential execution
- **Conditional Logic**: Complex control flow
- **Evaluator Reuse**: Multiple evaluations with same evaluator
- **Error Handling**: Type errors and evaluation failures

All tests pass with zero memory leaks.

## Integration with Interpreter

The method evaluator is designed to integrate seamlessly with the interpreter:

1. Interpreter stores parsed methods
2. When executing a method, creates a method evaluator
3. Creates a frame with current execution context
4. Evaluates the method with the frame
5. Reuses evaluator for multiple method calls

This enables efficient method execution without reparsing.

## Future Enhancements

- Performance profiling and optimization
- Parallel instruction execution where safe
- Debugging support with breakpoints
- Instruction-level error recovery
- Method caching strategies