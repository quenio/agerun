# Condition Instruction Evaluator Module

## Overview

The condition instruction evaluator module is responsible for evaluating if/else instructions in the AgeRun language. It handles conditional execution based on boolean expressions.

## Purpose

This module extracts the conditional instruction evaluation logic from the main instruction evaluator, following the single responsibility principle. It provides specialized handling for control flow with proper instruction list execution.

## Key Components

### Public Interface

```c
bool ar_condition_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);
```

### Functionality

The module evaluates if instructions of the form:
- `if(condition) { instructions }`
- `if(condition) { instructions } else { instructions }`

Key features:
1. **Condition Evaluation**: Evaluates the condition expression
2. **Truthiness Rules**: 
   - Integers: 0 is false, non-zero is true
   - Strings: empty is false, non-empty is true
   - Other types: false
3. **Branch Selection**: Executes then-branch if true, else-branch if false
4. **Instruction List Execution**: Evaluates all instructions in selected branch

### Memory Management

The module follows strict memory ownership rules:
- Condition evaluation results are owned and must be destroyed
- Each instruction in the selected branch is evaluated
- Proper cleanup on all code paths

## Dependencies

- `agerun_expression_evaluator`: For evaluating expressions
- `agerun_expression_parser`: For parsing expression strings
- `agerun_expression_ast`: For expression AST nodes
- `agerun_instruction_ast`: For accessing instruction AST structure
- `agerun_data`: For data manipulation
- `agerun_string`: For string operations
- `agerun_heap`: For memory tracking

## Implementation Details

The module:
1. Extracts and evaluates the condition expression
2. Determines truthiness based on data type and value
3. Selects appropriate instruction list (then or else)
4. Creates instruction evaluator for executing the branch
5. Evaluates each instruction in sequence
6. Returns true only if all instructions succeed

## Usage Example

```c
// Create evaluator
expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);

// Parse if instruction: if(memory.count > 0) { send(1, "positive") }
instruction_ast_t *ast = ar__instruction_parser__parse_if(parser);

// Evaluate the condition
bool success = ar_condition_instruction_evaluator__evaluate(
    expr_eval, memory, ast
);

// Instructions in the appropriate branch have been executed
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