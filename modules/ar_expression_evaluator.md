# Expression Evaluator Module

**Note**: This module has been migrated to Zig for improved memory safety and cleaner error handling patterns.

## Overview

The expression evaluator module provides functionality for evaluating expression AST nodes to produce data values. It serves as the execution engine for the expression system, taking parsed AST structures and evaluating them using frame context to produce results.

## Key Features

- **AST-Based Evaluation**: Evaluates expression AST nodes created by the expression parser
- **Frame-Based Execution**: Uses frame context for memory and context access
- **Opaque Evaluator Type**: Uses an opaque structure to encapsulate evaluation state
- **Single Evaluation Function**: Unified interface that handles all expression types internally
- **Memory and Context Access**: Supports evaluation of memory.x and context.x paths via frame
- **Container Literals**: Evaluates list and map literal AST nodes into fresh ar_data_t containers
- **Pure Function Calls**: Evaluates registered pure calls such as `parse(...)`, `build(...)`,
  `if(...)`, `append(...)`, `head(...)`, and `tail(...)` into fresh values
- **Binary Operations**: Implements all arithmetic and comparison operators
- **Recursive Evaluation**: Properly handles nested expressions through recursion
- **Ownership Semantics**: Clear distinction between references (memory access) and owned values (operations)

## Architecture

The module is designed with clean separation of concerns:
- Depends on ar_expression_ast for node inspection
- Depends on data module for value creation and manipulation  
- Depends on ar_pure_call for registered pure-call classification
- Depends on ar_log for centralized error reporting
- Depends on ar_frame for accessing memory, context, and message
- No dependency on expression or instruction modules
- Stateless evaluation - memory and context accessed through frame parameter

## API Functions

### Creation and Destruction

- `ar_expression_evaluator__create()` - Creates evaluator with log reference
- `ar_expression_evaluator__destroy()` - Destroys evaluator and releases resources

### Evaluation Function

- `ar_expression_evaluator__evaluate()` - Evaluates any expression AST node using frame context
  - Takes const evaluator instance, frame reference, and AST reference
  - Returns evaluated result with ownership semantics based on expression type
  - Internally dispatches to appropriate evaluation logic based on AST node type

## Ownership Model

The evaluator follows strict ownership semantics:
- **Frame Parameter**: Borrowed reference, not owned by evaluator
- **Memory Access Results**: Returns borrowed references to existing values in frame's memory/context
- **Literal Results**: Returns new owned values that caller must destroy
- **Literal Container Results**: Returns new list/map values containing owned evaluated child values
- **Pure Call Results**: Return new owned values; `parse(...)` returns a new MAP, `build(...)`
  returns a new STRING, `if(...)` returns the selected branch value or integer `0`, `append(...)`
  returns a new LIST or integer `0`, `head(...)` returns a copied item or integer `0`, and
  `tail(...)` returns a new LIST or integer `0` when allocation succeeds
- **Operation Results**: Returns new owned values that caller must destroy
- **Binary Operations**: Creates new values for all arithmetic and comparison results

## Type System

Supports evaluation of all AgeRun data types:
- **Integers**: Native integer operations and comparisons
- **Doubles**: Floating-point operations with automatic type promotion
- **Strings**: Concatenation (with +) and comparison operations
- **Lists**: Literal construction with evaluated item values
- **Maps**: Navigation through nested structures via memory access and literal construction with identifier keys

## Pure Function Calls

The evaluator supports registered pure function calls through generic `CALL` AST nodes. Each call is
first classified through `ar_pure_call`, then evaluated by the matching pure operation. Pure calls
evaluate their arguments using ordinary expression semantics and pass the resulting values to shared
pure operations.

For `parse(...)`:
- STRING, INTEGER, and DOUBLE arguments are interpreted as strings.
- Missing values, LIST or MAP arguments, malformed templates, and non-matching input return an empty MAP.
- Placeholder names are ordinary result keys; `self` and nested `self.*` fields are allowed in the
  returned map when storage rules allow the destination.

For `build(...)`:
- STRING, INTEGER, and DOUBLE templates are interpreted as strings.
- Missing or non-primitive templates use an empty string fallback.
- Placeholders are substituted from MAP values when the matched value is STRING, INTEGER, or DOUBLE.
- Missing placeholders, non-MAP values arguments, and non-primitive placeholder values are preserved
  unchanged in the returned STRING.

For `if(...)`:
- The condition is evaluated first using shared AgeRun condition truthiness.
- Integer `0`, non-integer values, and missing condition values select the false branch.
- Nonzero integers select the true branch.
- Only the selected branch expression is evaluated; the unselected branch is never evaluated.
- If the selected branch cannot produce a value, expression-level `if(...)` returns integer `0`.
- Argument handling is path-neutral; storage rules, not argument paths, protect `memory.self`.

For `append(...)`:
- LIST values return a new LIST containing deep copies of every source item followed by a deep copy
  of the appended value.
- Empty LIST values return a new one-item LIST.
- Missing values, non-LIST list inputs, and copy failures return integer `0`.
- The source list and appended value are never mutated.
- Argument handling is path-neutral; storage rules, not argument paths, protect `memory.self`.

For `head(...)`:
- LIST values with at least one item return a deep copy of the first item.
- Empty LIST values, missing values, non-LIST values, and copy failures return integer `0`.
- Argument handling is path-neutral; storage rules, not argument paths, protect `memory.self`.

For `tail(...)`:
- LIST values return a new LIST containing deep copies of every item after the first.
- Empty and single-item LIST values return a new empty LIST.
- Missing values, non-LIST values, and copy failures return integer `0`.
- Argument handling is path-neutral; storage rules, not argument paths, protect `memory.self`.

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
- List equality is currently limited to empty-list checks such as `message.items = []` and
  `message.items <> []`; integer `0` is not equal to `[]`. `tail([])` and `tail([value])`
  produce an empty LIST, while missing or non-LIST `tail(...)` inputs store integer `0`. Methods
  that accept possibly invalid list inputs should normalize `tail(...) = 0` to `[]` before using
  `... = []` as a traversal stop. Non-empty lists are not structurally compared.

## Error Handling

The module provides comprehensive error handling:
- NULL checks for all parameters
- Type validation for node-specific functions
- Division by zero detection
- Missing memory key handling
- Invalid base accessor detection

## Usage Example

```c
// Create log and evaluator
ar_log_t *log = ar_log__create();
ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log);

// Create frame with memory and context
ar_data_t *memory = ar_data__create_map();
ar_data_t *context = ar_data__create_map();
ar_data_t *message = ar_data__create_string("test");
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Set some values in memory
ar_data__set_map_integer(memory, "x", 10);

// Parse an expression to AST
ar_expression_parser_t *parser = ar_expression_parser__create(log);
ar_expression_ast_t *ast = ar_expression_parser__parse(parser, "memory.x + 5");

// Evaluate the AST using the frame
ar_data_t *result = ar_expression_evaluator__evaluate(evaluator, frame, ast);

// Clean up
ar_data__destroy(result);  // Owned value from operation
ar_expression_ast__destroy(ast);
ar_expression_parser__destroy(parser);
ar_frame__destroy(frame);
ar_expression_evaluator__destroy(evaluator);
ar_data__destroy(memory);
ar_data__destroy(context);
ar_data__destroy(message);
ar_log__destroy(log);
```

## Implementation Notes

- All type-specific evaluation functions are internal (static) implementation details
- Main `evaluate` function dispatches to appropriate internal evaluator based on AST node type
- Uses a helper function `_evaluate_expression()` for recursive evaluation
- List and map literal evaluation recursively evaluates child expressions and transfers them into the new container
- Memory access results are copied when used in binary operations to ensure ownership safety
- String concatenation allocates temporary buffer for result
- Frame-based execution allows stateless evaluation - no stored memory/context references
- Maintains zero memory leaks through careful ownership tracking
