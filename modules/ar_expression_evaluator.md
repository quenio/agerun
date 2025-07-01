# Expression Evaluator Module

## Overview

The expression evaluator module provides functionality for evaluating expression AST nodes to produce data values. It serves as the execution engine for the expression system, taking parsed AST structures and evaluating them against memory and context maps to produce results.

## Key Features

- **AST-Based Evaluation**: Evaluates expression AST nodes created by the expression parser
- **Opaque Evaluator Type**: Uses an opaque structure to encapsulate evaluation state
- **Type-Specific Functions**: Provides separate evaluation functions for each AST node type
- **Memory and Context Access**: Supports evaluation of memory.x and context.x paths
- **Binary Operations**: Implements all arithmetic and comparison operators
- **Recursive Evaluation**: Properly handles nested expressions through recursion
- **Ownership Semantics**: Clear distinction between references (memory access) and owned values (operations)

## Architecture

The module is designed with clean separation of concerns:
- Depends on ar_expression_ast for node inspection
- Depends on data module for value creation and manipulation  
- Depends on ar_log for centralized error reporting
- No dependency on expression or instruction modules
- Maintains memory and context references throughout evaluation

## API Functions

### Creation and Destruction

- `ar_expression_evaluator__create()` - Creates evaluator with log, memory and optional context
- `ar_expression_evaluator__destroy()` - Destroys evaluator and releases resources

### Evaluation Functions

- `ar_expression_evaluator__evaluate_literal_int()` - Evaluates integer literal nodes
- `ar_expression_evaluator__evaluate_literal_double()` - Evaluates double literal nodes
- `ar_expression_evaluator__evaluate_literal_string()` - Evaluates string literal nodes
- `ar_expression_evaluator__evaluate_memory_access()` - Evaluates memory/context access paths
- `ar_expression_evaluator__evaluate_binary_op()` - Evaluates binary operations recursively

## Ownership Model

The evaluator follows strict ownership semantics:
- **Memory and Context**: Holds borrowed references, does not own
- **Memory Access Results**: Returns borrowed references to existing values
- **Operation Results**: Returns new owned values that caller must destroy
- **Binary Operations**: Creates new values for all arithmetic and comparison results

## Type System

Supports evaluation of all AgeRun data types:
- **Integers**: Native integer operations and comparisons
- **Doubles**: Floating-point operations with automatic type promotion
- **Strings**: Concatenation (with +) and comparison operations
- **Maps**: Navigation through nested structures via memory access

## Binary Operations

Implements the full set of AgeRun operators:

### Arithmetic Operators
- Addition (+) - numeric addition or string concatenation
- Subtraction (-) - numeric only
- Multiplication (*) - numeric only  
- Division (/) - numeric only with zero-check

### Comparison Operators
- Equal (=) - all types
- Not Equal (<>) - all types
- Less Than (<) - numeric only
- Greater Than (>) - numeric only
- Less Than or Equal (<=) - numeric only
- Greater Than or Equal (>=) - numeric only

## Type Conversions

The evaluator handles automatic type promotion:
- If either operand is a double, both are converted to double
- Integer operations produce integer results
- Double operations produce double results
- Comparison operations always produce integer results (0 or 1)

## Error Handling

The module provides comprehensive error handling:
- NULL checks for all parameters
- Type validation for node-specific functions
- Division by zero detection
- Missing memory key handling
- Invalid base accessor detection

## Usage Example

```c
// Create evaluator with memory and context
data_t *memory = ar_data__create_map();
data_t *context = ar_data__create_map();
ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(memory, context);

// Parse an expression to AST
ar_expression_parser_t *parser = ar__expression_parser__create("memory.x + 5");
ar_expression_ast_t *ast = ar__expression_parser__parse(parser);

// Evaluate the AST
data_t *result = ar_expression_evaluator__evaluate_binary_op(evaluator, ast);

// Clean up
ar_data__destroy(result);  // Owned value from operation
ar_expression_ast__destroy(ast);
ar__expression_parser__destroy(parser);
ar_expression_evaluator__destroy(evaluator);
ar_data__destroy(memory);
ar_data__destroy(context);
```

## Implementation Notes

- Uses a helper function `_evaluate_expression()` for recursive evaluation
- Memory access results are copied when used in binary operations
- String concatenation allocates temporary buffer for result
- All evaluation functions check node type before proceeding
- Maintains zero memory leaks through careful ownership tracking