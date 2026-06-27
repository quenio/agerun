# ar_method_evaluator

The method evaluator module provides functionality to evaluate method AST nodes using a frame-based execution model. It serves as the top-level evaluator that coordinates the execution of all instructions within a method.

## Overview

The method evaluator is responsible for:
- Evaluating complete methods represented as ASTs
- Processing all instructions in sequence
- Using frames to pass execution context
- Attempting every instruction in a method definition once evaluation starts

## Key Features

- **Frame-based execution**: Uses frames to bundle memory, context, and message
- **Sequential evaluation**: Processes instructions in order (1-based indexing)
- **Full instruction pass**: Logs failed instructions and continues to later instructions
- **Top-down design**: First evaluator to use frames, establishing the pattern

## API Functions

### ar_method_evaluator__create
```c
ar_method_evaluator_t* ar_method_evaluator__create(
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
);
```
Creates a new method evaluator with the given log, agency, and delegation.
- **Ownership**: Returns owned value that caller must destroy
- **Parameters**: Borrows references to log, agency, and delegation
- **Behavior**: Creates and owns its instruction evaluator internally, passing all dependencies

### ar_method_evaluator__destroy
```c
void ar_method_evaluator__destroy(
    ar_method_evaluator_t *own_evaluator
);
```
Destroys a method evaluator and frees its resources.
- **Ownership**: Takes ownership and destroys the evaluator

### ar_method_evaluator__evaluate
```c
bool ar_method_evaluator__evaluate(
    const ar_method_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_method_ast_t *ref_ast
);
```
Evaluates a method AST using the provided frame.
- **Returns**: true if all instructions succeed, false if any instruction fails
- **Behavior**: 
  - Empty methods return true
  - Evaluates each instruction in sequence
  - Attempts every instruction even when an earlier instruction fails

## Implementation Details

The method evaluator:
1. Validates inputs (evaluator, frame, and AST must not be NULL)
2. Gets the instruction count from the method AST
3. For empty methods, returns true immediately
4. Iterates through instructions using 1-based indexing
5. Evaluates each instruction via the instruction evaluator
6. Logs errors with line numbers when instructions fail
7. Continues after instruction failures so later instructions are still evaluated
8. Returns false after the full pass if any instruction fails or cannot be retrieved
9. Returns true only if all instructions succeed

## Error Handling

The method evaluator provides detailed error reporting:
- Logs error when instruction retrieval fails (with line number)
- Logs error when instruction evaluation fails (with line number)
- Error messages include the specific line where the failure occurred
- Uses the ar_log module for centralized error collection
- Instruction failures do not stop the method pass; later instructions are still attempted

## Usage Example

```c
// Create method evaluator with log, agency, and delegation
ar_log_t *log = ar_log__create();
ar_agency_t *agency = /* from system */;
ar_delegation_t *delegation = /* from system */;
ar_method_evaluator_t *method_eval = ar_method_evaluator__create(log, agency, delegation);

// Create frame with execution context
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate a method AST
ar_method_ast_t *method_ast = /* parsed method */;
bool success = ar_method_evaluator__evaluate(method_eval, frame, method_ast);

// Cleanup
ar_method_evaluator__destroy(method_eval);
ar_frame__destroy(frame);
ar_log__destroy(log);
```

## Design Notes

- This is the first evaluator to use frames, establishing the pattern for frame-based execution
- The method evaluator creates and owns its instruction evaluator internally
- The instruction evaluator is stored in the struct and used for evaluating each instruction
- Provides a simplified API that only requires a log instance
- Once method evaluation starts with a valid evaluator, frame, and method AST, the evaluator attempts
  every instruction in source order before returning

## Dependencies

- ar_log: For error reporting
- ar_instruction_evaluator: Created and owned internally for evaluating instructions
- ar_method_ast: For method representation
- ar_frame: For execution context
- ar_heap: For memory allocation (via Zig's C imports)

## Implementation Language

This module is implemented in Zig with full C ABI compatibility, following AgeRun's pattern for new modules.
