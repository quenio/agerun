# Instruction Evaluator Module

The instruction evaluator module (`agerun_instruction_evaluator`) is responsible for executing parsed instruction AST nodes within the AgeRun runtime environment. It evaluates all instruction types defined in the AgeRun language, managing memory operations, agent interactions, and method executions.

## Purpose

The instruction evaluator serves as the execution engine for AgeRun instructions, bridging the gap between parsed AST representations and actual runtime operations. It:

- Evaluates expressions using the expression evaluator module
- Manages memory assignments and data manipulation
- Handles agent creation, destruction, and messaging
- Processes method creation and registration
- Implements control flow (conditional execution)
- Performs template-based string operations (parse/build)

## Architecture

### Key Components

1. **Evaluator Context**: Maintains references to memory, context, message, and expression evaluator
2. **Individual Evaluation Functions**: Separate functions for each instruction type
3. **Helper Functions**: Common patterns for expression evaluation, argument extraction, and result storage

### Design Principles

- **Separation of Concerns**: Parsing (instruction_parser) is separate from evaluation
- **No Circular Dependencies**: Takes dependencies as parameters rather than importing high-level modules
- **Memory Safety**: Strict ownership semantics with proper cleanup
- **Extensibility**: Easy to add new instruction types

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions within instructions
- `agerun_instruction_ast`: For accessing parsed instruction structures
- `agerun_data`: For data manipulation and storage
- `agerun_agency`: For agent operations (create, destroy, send)
- `agerun_methodology`: For method operations
- `agerun_method`: For method creation
- `agerun_destroy_agent_instruction_evaluator`: For agent destruction
- `agerun_destroy_method_instruction_evaluator`: For method destruction
- `agerun_list`: For argument counting in destroy dispatch
- `agerun_heap`: For memory tracking

## API Reference

### Types

```c
typedef struct instruction_evaluator_s instruction_evaluator_t;
```

An opaque type representing an instruction evaluator instance.

### Lifecycle Functions

#### ar__instruction_evaluator__create

```c
instruction_evaluator_t* ar__instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
);
```

Creates a new instruction evaluator instance.

**Parameters:**
- `ref_expr_evaluator`: Expression evaluator to use (required, borrowed reference)
- `mut_memory`: Memory map for storing values (required, mutable reference)
- `ref_context`: Optional context map (can be NULL, borrowed reference)
- `ref_message`: Optional message data (can be NULL, borrowed reference)

**Returns:** New evaluator instance or NULL on failure

**Ownership:** Caller owns the returned evaluator and must destroy it

#### ar__instruction_evaluator__destroy

```c
void ar__instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator);
```

Destroys an instruction evaluator instance.

**Parameters:**
- `own_evaluator`: The evaluator to destroy (owned, can be NULL)

### Evaluation Functions

Each evaluation function follows the same pattern:
- Takes the evaluator and an instruction AST node
- Returns `true` on successful evaluation, `false` on failure
- May modify memory through assignments
- Handles result storage if the instruction has an assignment target

#### ar__instruction_evaluator__evaluate_assignment

```c
bool ar__instruction_evaluator__evaluate_assignment(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates assignment instructions (e.g., `memory.x := 42`).

**Behavior:**
- Evaluates the expression on the right side
- Stores the result at the specified memory path
- Supports nested paths (e.g., `memory.user.name`)
- Requires paths to start with "memory."

#### ar__instruction_evaluator__evaluate_send

```c
bool ar__instruction_evaluator__evaluate_send(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates send instructions (e.g., `send(agent_id, message)`).

**Behavior:**
- Evaluates agent ID and message expressions
- Sends message to specified agent
- Returns success/failure result
- Special case: `send(0, msg)` is a no-op that returns true

#### ar__instruction_evaluator__evaluate_if

```c
bool ar__instruction_evaluator__evaluate_if(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates conditional instructions (e.g., `if(condition, true_expr, false_expr)`).

**Behavior:**
- Evaluates condition expression
- Non-zero integers are true, zero is false
- Only evaluates the selected branch
- Returns the result of the evaluated branch

#### ar__instruction_evaluator__evaluate_parse

```c
bool ar__instruction_evaluator__evaluate_parse(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates parse instructions (e.g., `parse("Name: {name}, Age: {age}", input)`).

**Behavior:**
- Extracts values from input string based on template
- Automatically detects types (integer, double, string)
- Returns a map with extracted values
- Returns empty map if template doesn't match

#### ar__instruction_evaluator__evaluate_build

```c
bool ar__instruction_evaluator__evaluate_build(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates build instructions (e.g., `build("Hello {name}!", values_map)`).

**Behavior:**
- Replaces placeholders in template with values from map
- Converts all types to strings
- Preserves placeholders for missing values
- Returns the built string

#### ar__instruction_evaluator__evaluate_method

```c
bool ar__instruction_evaluator__evaluate_method(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates method creation instructions (e.g., `method("greet", "send(0, \"Hello!\")", "1.0.0")`).

**Behavior:**
- Creates a new method with given name, instructions, and version
- Registers method with methodology
- Returns 1 on success, 0 on failure
- Does not validate method instructions (validated at execution)

#### ar__instruction_evaluator__evaluate_agent

```c
bool ar__instruction_evaluator__evaluate_agent(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates agent creation instructions (e.g., `agent("echo", "1.0.0", context_map)`).

**Behavior:**
- Creates a new agent with specified method and context
- Validates method exists before creation
- Returns agent ID on success, 0 on failure
- Currently requires all 3 parameters (context cannot be omitted)

#### ar__instruction_evaluator__evaluate_destroy

```c
bool ar__instruction_evaluator__evaluate_destroy(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);
```

Evaluates destroy instructions for agents and methods by dispatching to the appropriate specialized evaluator based on argument count.

**Behavior:**
- `destroy(agent_id)`: Dispatches to `destroy_agent_instruction_evaluator` (1 argument)
- `destroy(method_name, version)`: Dispatches to `destroy_method_instruction_evaluator` (2 arguments)
- The agent evaluator destroys the specified agent
- The method evaluator sends `__sleep__` messages before destroying agents using the method
- Returns true on success, false on failure or invalid argument count

## Usage Examples

### Basic Assignment

```c
// Create evaluator
instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
    expr_eval, memory, NULL, NULL
);

// Create assignment AST: memory.count := 10
instruction_ast_t *ast = ar__instruction_ast__create_assignment(
    "memory.count", "10"
);

// Evaluate
bool success = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);

// Clean up
ar__instruction_ast__destroy(ast);
ar__instruction_evaluator__destroy(evaluator);
```

### Conditional Execution

```c
// Create if AST: memory.result := if(memory.x > 5, "big", "small")
const char *args[] = {"memory.x > 5", "\"big\"", "\"small\""};
instruction_ast_t *ast = ar__instruction_ast__create_function_call(
    INST_AST_IF, "if", args, 3, "memory.result"
);

// Evaluate
bool success = ar__instruction_evaluator__evaluate_if(evaluator, ast);
```

### Template Operations

```c
// Parse example
const char *parse_args[] = {
    "\"User: {name}, Age: {age}\"",
    "\"User: Alice, Age: 30\""
};
instruction_ast_t *parse_ast = ar__instruction_ast__create_function_call(
    INST_AST_PARSE, "parse", parse_args, 2, "memory.parsed"
);
ar__instruction_evaluator__evaluate_parse(evaluator, parse_ast);

// Build example
const char *build_args[] = {
    "\"Hello {name}!\"",
    "memory.parsed"
};
instruction_ast_t *build_ast = ar__instruction_ast__create_function_call(
    INST_AST_BUILD, "build", build_args, 2, "memory.greeting"
);
ar__instruction_evaluator__evaluate_build(evaluator, build_ast);
```

## Memory Management

### Ownership Rules

1. **Evaluator Ownership**: The evaluator does not take ownership of any parameters passed to create()
2. **Expression Results**: Expression evaluation returns owned values that must be destroyed
3. **Memory References**: Memory access expressions return references (not owned)
4. **Agent Context**: For agent creation, context is borrowed (not transferred)
5. **Send Messages**: Messages sent to agents transfer ownership to the agency

### Common Patterns

The module uses several helper functions to manage memory correctly:

- `_parse_and_evaluate_expression()`: Evaluates expressions and returns owned values
- `_extract_function_args()`: Extracts arguments from AST with proper cleanup
- `_copy_data_value()`: Creates deep copies when needed
- `_store_result_if_assigned()`: Handles result storage with ownership transfer

## Error Handling

The evaluator provides comprehensive error checking:

- Validates AST node types before evaluation
- Checks argument counts and types
- Validates memory paths start with "memory."
- Reports errors through return values (true/false)
- Ensures partial operations are rolled back on failure

## Performance Considerations

- Expression parsing is done on-demand for each evaluation
- No caching of parsed expressions (stateless evaluation)
- Deep copying used for complex data structures when needed
- Efficient memory path resolution using string operations

## Testing

The module has comprehensive test coverage split into focused test files:

- `agerun_instruction_evaluator_tests.c`: Core create/destroy tests
- `agerun_assignment_instruction_evaluator_tests.c`: Assignment tests
- `agerun_send_instruction_evaluator_tests.c`: Message sending tests
- `agerun_condition_instruction_evaluator_tests.c`: If conditional tests
- `agerun_parse_instruction_evaluator_tests.c`: Parse function tests
- `agerun_build_instruction_evaluator_tests.c`: Build function tests
- `agerun_method_instruction_evaluator_tests.c`: Method creation tests
- `agerun_agent_instruction_evaluator_tests.c`: Agent creation tests
- `agerun_destroy_agent_instruction_evaluator_tests.c`: Agent destruction tests
- `agerun_destroy_method_instruction_evaluator_tests.c`: Method destruction tests

All tests follow TDD methodology with Given/When/Then structure and zero memory leaks.

## Future Enhancements

- Support optional parameters (e.g., agent creation without context)
- Add support for memory references in send() without copying
- Implement expression result caching for performance
- Add detailed error messages with position information
- Support for custom evaluation contexts