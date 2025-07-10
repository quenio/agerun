# ar_method_evaluator

The method evaluator module provides functionality to evaluate method AST nodes using a frame-based execution model. It serves as the top-level evaluator that coordinates the execution of all instructions within a method.

## Overview

The method evaluator is responsible for:
- Evaluating complete methods represented as ASTs
- Processing all instructions in sequence
- Using frames to pass execution context
- Stopping execution on the first error

## Key Features

- **Frame-based execution**: Uses frames to bundle memory, context, and message
- **Sequential evaluation**: Processes instructions in order (1-based indexing)
- **Error propagation**: Stops on first failed instruction
- **Top-down design**: First evaluator to use frames, establishing the pattern

## API Functions

### ar_method_evaluator__create
```c
ar_method_evaluator_t* ar_method_evaluator__create(
    ar_log_t *ref_log,
    ar_instruction_evaluator_t *ref_instruction_evaluator
);
```
Creates a new method evaluator with the given log and instruction evaluator.
- **Ownership**: Returns owned value that caller must destroy
- **Parameters**: Borrows references to log and instruction evaluator

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
    ar_method_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_method_ast_t *ref_ast
);
```
Evaluates a method AST using the provided frame.
- **Returns**: true if all instructions succeed, false on any error
- **Behavior**: 
  - Empty methods return true
  - Evaluates each instruction in sequence
  - Stops on first failure

## Implementation Details

The method evaluator:
1. Validates inputs (evaluator, frame, and AST must not be NULL)
2. Gets the instruction count from the method AST
3. For empty methods, returns true immediately
4. Iterates through instructions using 1-based indexing
5. Evaluates each instruction via the instruction evaluator
6. Logs errors with line numbers when instructions fail
7. Returns false if any instruction fails or cannot be retrieved
8. Returns true only if all instructions succeed

## Error Handling

The method evaluator provides detailed error reporting:
- Logs error when instruction retrieval fails (with line number)
- Logs error when instruction evaluation fails (with line number)
- Error messages include the specific line where the failure occurred
- Uses the ar_log module for centralized error collection

## Usage Example

```c
// Create evaluators
ar_log_t *log = ar_log__create();
ar_instruction_evaluator_t *instr_eval = /* create instruction evaluator */;
ar_method_evaluator_t *method_eval = ar_method_evaluator__create(log, instr_eval);

// Create frame with execution context
ar_frame_t *frame = ar_frame__create(memory, context, message);

// Evaluate a method AST
ar_method_ast_t *method_ast = /* parsed method */;
bool success = ar_method_evaluator__evaluate(method_eval, frame, method_ast);

// Cleanup
ar_method_evaluator__destroy(method_eval);
ar_frame__destroy(frame);
```

## Design Notes

- This is the first evaluator to use frames, establishing the pattern for frame-based execution
- The instruction evaluator hasn't been updated to use frames yet, but works because it was initialized with the same memory/context/message that are in the frame
- Future work will update instruction and expression evaluators to accept frames directly

## Dependencies

- ar_log: For error reporting
- ar_instruction_evaluator: For evaluating individual instructions
- ar_method_ast: For method representation
- ar_frame: For execution context
- ar_heap: For memory allocation (via Zig's C imports)

## Implementation Language

This module is implemented in Zig with full C ABI compatibility, following AgeRun's pattern for new modules.