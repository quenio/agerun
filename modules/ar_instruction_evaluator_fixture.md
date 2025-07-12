# Instruction Evaluator Fixture Module

The instruction evaluator fixture module (`ar_instruction_evaluator_fixture`) provides reusable test infrastructure for testing instruction evaluators. It manages common test resources and provides helper functions to simplify test creation.

## Purpose

This module serves as a test fixture that:
- Manages test resources (memory, log, expression evaluator, frames)
- Provides helper functions for creating test ASTs
- Tracks allocated resources for proper cleanup
- Simplifies instruction evaluator test implementation

## Key Features

### Resource Management

The fixture automatically manages:
- Test name storage
- Log instance for error tracking
- Memory map for test data
- Expression evaluator
- Tracking of frames, ASTs, and data objects for cleanup

### Helper Functions

Provides convenience functions for:
- Creating frames with test data
- Creating assignment ASTs with various expression types
- Creating literal expressions (integers, strings)
- Accessing managed resources

## API Reference

### Types

```c
typedef struct ar_instruction_evaluator_fixture_s ar_instruction_evaluator_fixture_t;
```

An opaque type representing the test fixture instance.

### Lifecycle Functions

#### ar_instruction_evaluator_fixture__create

```c
ar_instruction_evaluator_fixture_t* ar_instruction_evaluator_fixture__create(
    const char *ref_test_name
);
```

Creates a new test fixture instance with all necessary resources.

**Parameters:**
- `ref_test_name`: Name of the test for logging purposes

**Returns:** New fixture instance or NULL on failure

**Ownership:** Caller owns the returned fixture and must destroy it

#### ar_instruction_evaluator_fixture__destroy

```c
void ar_instruction_evaluator_fixture__destroy(
    ar_instruction_evaluator_fixture_t *own_fixture
);
```

Destroys the fixture and all tracked resources.

**Parameters:**
- `own_fixture`: The fixture to destroy (owned, can be NULL)

### Resource Access Functions

#### ar_instruction_evaluator_fixture__get_log

```c
ar_log_t* ar_instruction_evaluator_fixture__get_log(
    const ar_instruction_evaluator_fixture_t *ref_fixture
);
```

Returns the log instance for error checking.

#### ar_instruction_evaluator_fixture__get_memory

```c
ar_data_t* ar_instruction_evaluator_fixture__get_memory(
    const ar_instruction_evaluator_fixture_t *ref_fixture
);
```

Returns the memory map for test data storage.

#### ar_instruction_evaluator_fixture__get_expression_evaluator

```c
ar_expression_evaluator_t* ar_instruction_evaluator_fixture__get_expression_evaluator(
    const ar_instruction_evaluator_fixture_t *ref_fixture
);
```

Returns the expression evaluator for test use.

### Helper Functions

#### ar_instruction_evaluator_fixture__create_frame

```c
ar_frame_t* ar_instruction_evaluator_fixture__create_frame(
    ar_instruction_evaluator_fixture_t *mut_fixture
);
```

Creates a new frame with default context and message.

**Returns:** New frame instance (tracked by fixture)

**Note:** The frame is automatically cleaned up when the fixture is destroyed

#### ar_instruction_evaluator_fixture__create_assignment_int

```c
ar_instruction_ast_t* ar_instruction_evaluator_fixture__create_assignment_int(
    ar_instruction_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    int value
);
```

Creates an assignment AST with an integer literal expression.

**Parameters:**
- `ref_path`: Target path (e.g., "memory.x")
- `value`: Integer value to assign

**Returns:** New assignment AST (tracked by fixture)

#### ar_instruction_evaluator_fixture__create_assignment_string

```c
ar_instruction_ast_t* ar_instruction_evaluator_fixture__create_assignment_string(
    ar_instruction_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    const char *ref_value
);
```

Creates an assignment AST with a string literal expression.

**Parameters:**
- `ref_path`: Target path (e.g., "memory.name")
- `ref_value`: String value to assign

**Returns:** New assignment AST (tracked by fixture)

#### ar_instruction_evaluator_fixture__create_assignment_expr

```c
ar_instruction_ast_t* ar_instruction_evaluator_fixture__create_assignment_expr(
    ar_instruction_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    ar_expression_ast_t *own_expr
);
```

Creates an assignment AST with a custom expression.

**Parameters:**
- `ref_path`: Target path
- `own_expr`: Expression AST (ownership transferred)

**Returns:** New assignment AST (tracked by fixture)

## Usage Examples

### Basic Test Structure

```c
static void test_assignment_evaluator__evaluate_integer(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *fixture = 
        ar_instruction_evaluator_fixture__create("test_evaluate_integer");
    assert(fixture != NULL);
    
    // When creating an assignment: memory.count := 42
    ar_instruction_ast_t *ast = ar_instruction_evaluator_fixture__create_assignment_int(
        fixture, "memory.count", 42
    );
    assert(ast != NULL);
    
    // Create a frame for evaluation
    ar_frame_t *frame = ar_instruction_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Get resources for evaluator creation
    ar_log_t *log = ar_instruction_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = 
        ar_instruction_evaluator_fixture__get_expression_evaluator(fixture);
    
    // Create and test evaluator
    ar_assignment_instruction_evaluator_t *evaluator = 
        ar_assignment_instruction_evaluator__create(log, expr_eval);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    assert(result == true);
    
    // Verify result
    ar_data_t *memory = ar_instruction_evaluator_fixture__get_memory(fixture);
    assert(ar_data__get_map_integer(memory, "count") == 42);
    
    // Cleanup (fixture handles AST and frame cleanup)
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_instruction_evaluator_fixture__destroy(fixture);
}
```

### Custom Expression Test

```c
// Create custom expression AST
ar_expression_ast_t *expr = ar_expression_ast__create_binary_op(
    AR_BINARY_OPERATOR__ADD,
    ar_expression_ast__create_literal_int(10),
    ar_expression_ast__create_literal_int(20)
);

// Create assignment with expression
ar_instruction_ast_t *ast = ar_instruction_evaluator_fixture__create_assignment_expr(
    fixture, "memory.sum", expr
);
```

## Memory Management

The fixture automatically tracks and cleans up:
- All created frames
- All created ASTs
- All created data objects
- Internal resources (log, memory, expression evaluator)

This ensures no memory leaks in tests even if assertions fail.

## Design Principles

1. **Reusability**: Common test patterns extracted into helper functions
2. **Safety**: Automatic resource tracking prevents memory leaks
3. **Simplicity**: Clean API reduces boilerplate in tests
4. **Flexibility**: Can be extended with additional helper functions

## Testing

The fixture itself has tests in `ar_instruction_evaluator_fixture_tests.c` that verify:
- Resource creation and cleanup
- Helper function correctness
- Memory leak detection
- Error handling