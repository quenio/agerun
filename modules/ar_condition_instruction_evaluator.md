# Condition Instruction Evaluator Module

## Overview

The condition instruction evaluator module is responsible for evaluating if/condition instructions in the AgeRun language. It handles conditional expressions that select between two values based on a condition.

This module follows an instantiable design pattern where evaluators are created with their dependencies and can be reused for multiple evaluations.

## Purpose

This module extracts the conditional instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for ternary conditional expressions (if(condition, true_value, false_value)).

## Key Components

### Types

```c
typedef struct ar_condition_instruction_evaluator_s ar_condition_instruction_evaluator_t;
```

An opaque type representing a condition instruction evaluator instance.

### Public Interface

```c
ar_condition_instruction_evaluator_t* ar_condition_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);
```
Creates a new condition instruction evaluator that stores its dependencies.

```c
void ar_condition_instruction_evaluator__destroy(
    ar_condition_instruction_evaluator_t *own_evaluator
);
```
Destroys a condition instruction evaluator and frees all resources.

```c
bool ar_condition_instruction_evaluator__evaluate(
    ar_condition_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);
```
Evaluates a condition instruction using the stored dependencies.

### Functionality

The module evaluates conditional instructions of the form:
- `if(condition, true_expr, false_expr)`
- `memory.result := if(condition, true_expr, false_expr)`

Key features:
1. **Condition Evaluation**: Evaluates the condition expression to determine which branch to take
2. **Branch Selection**: Only evaluates the selected branch (true or false), not both
3. **Result Assignment**: Stores the evaluated value when assignment is specified
4. **Type Flexibility**: Can return any data type from either branch
5. **Short-circuit Evaluation**: Only the selected expression is evaluated

### Memory Management

The module follows strict memory ownership rules:
- The evaluator instance owns its internal structure but not the dependencies
- Expression evaluator and memory are borrowed references stored in the instance
- Condition evaluation results are owned and must be destroyed
- Branch evaluation results are owned by the caller when assigned
- The create function returns ownership to the caller
- The destroy function takes ownership and frees all resources

## Dependencies

- `ar_expression_evaluator`: For evaluating expressions
- `ar_expression_parser`: For parsing expression strings
- `ar_expression_ast`: For expression AST nodes
- `ar_instruction_ast`: For accessing instruction AST structure
- `ar_data`: For data manipulation
- `ar_string`: For string operations
- `ar_heap`: For memory tracking

## Implementation Details

The module evaluates all three arguments:
1. Condition must evaluate to an integer (0 = false, non-zero = true)
2. True expression is evaluated only if condition is true
3. False expression is evaluated only if condition is false
4. Uses helper functions for expression parsing and evaluation
5. Handles result storage for assigned conditionals

## Usage Example

```c
// Create memory and expression evaluator
data_t *memory = ar__data__create_map();
ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Create condition instruction evaluator
ar_condition_instruction_evaluator_t *cond_eval = ar__condition_instruction_evaluator__create(
    expr_eval, memory
);

// Parse if instruction: result := if(x > 5, 100, 200)
ar_instruction_ast_t *ast = ar__instruction_parser__parse_if(parser);

// Evaluate the condition
bool success = ar__condition_instruction_evaluator__evaluate(cond_eval, ast);

// The appropriate value (100 or 200) has been stored in memory.result

// Cleanup
ar__condition_instruction_evaluator__destroy(cond_eval);
ar__expression_evaluator__destroy(expr_eval);
ar__data__destroy(memory);
```

## Testing

The module includes comprehensive tests covering:
- True conditions (then-branch execution)
- False conditions (else-branch execution)
- Various condition types (integers, strings, expressions)
- Missing else branches
- Multiple instructions in branches
- Memory leak verification

All tests pass with zero memory leaks.