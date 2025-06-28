# Frame Module

## Overview

The frame module provides the execution context abstraction for AgeRun evaluators. A frame encapsulates the three essential components needed for evaluation: memory (agent state), context (creation-time configuration), and message (current input).

## Purpose

The frame module enables stateless evaluators by passing all necessary state as a single parameter rather than storing it in evaluator instances. This design allows evaluators to be created once and reused across all evaluations, improving performance and simplifying the architecture.

## Key Concepts

### Frame
A frame represents the complete execution context for evaluating expressions and instructions. It contains:
- **Memory**: The agent's mutable state (variables that can be read and written)
- **Context**: The agent's creation-time configuration (immutable data provided when the agent was created)
- **Message**: The current message being processed

All three components are required (non-NULL) to create a frame, ensuring evaluators always have a complete context for evaluation.

### Ownership Model
The frame holds borrowed references to memory, context, and message. It does not take ownership of these objects:
- The caller retains ownership of all three components
- The frame can be destroyed without affecting the referenced objects
- The referenced objects must remain valid for the lifetime of the frame

## API Functions

### ar_frame__create
```c
ar_frame_t* ar_frame__create(
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
);
```
Creates a new frame with the given memory, context, and message. All parameters must be non-NULL.

**Parameters:**
- `mut_memory`: The agent's memory (mutable reference, required)
- `ref_context`: The agent's context (borrowed reference, required)
- `ref_message`: The message being processed (borrowed reference, required)

**Returns:** A new frame instance, or NULL if any parameter is NULL

**Ownership:** Returns an owned value that caller must destroy

### ar_frame__destroy
```c
void ar_frame__destroy(ar_frame_t *own_frame);
```
Destroys a frame and releases its resources. This does not destroy the memory, context, or message objects.

**Parameters:**
- `own_frame`: The frame to destroy (ownership transferred)

**Ownership:** Takes ownership of the frame and destroys it. Safe to call with NULL.

### ar_frame__get_memory
```c
data_t* ar_frame__get_memory(const ar_frame_t *ref_frame);
```
Gets the memory from the frame.

**Parameters:**
- `ref_frame`: The frame to query (borrowed reference)

**Returns:** The memory map (mutable reference), never NULL for valid frames

**Ownership:** Returns a mutable reference. The caller should not destroy it.

### ar_frame__get_context
```c
const data_t* ar_frame__get_context(const ar_frame_t *ref_frame);
```
Gets the context from the frame.

**Parameters:**
- `ref_frame`: The frame to query (borrowed reference)

**Returns:** The context data (borrowed reference), never NULL for valid frames

**Ownership:** Returns a borrowed reference. The caller should not destroy it.

### ar_frame__get_message
```c
const data_t* ar_frame__get_message(const ar_frame_t *ref_frame);
```
Gets the message from the frame.

**Parameters:**
- `ref_frame`: The frame to query (borrowed reference)

**Returns:** The message data (borrowed reference), never NULL for valid frames

**Ownership:** Returns a borrowed reference. The caller should not destroy it.

## Design Decisions

### Non-NULL Requirement
All three components (memory, context, message) are required to be non-NULL. This decision simplifies evaluator code by eliminating the need for NULL checks throughout the evaluation pipeline. If an agent doesn't have a message or context, the caller should provide an empty map or string rather than NULL.

### Borrowed References
The frame does not take ownership of its components. This allows the same memory, context, and message objects to be used across multiple evaluations without copying, improving performance.

### Stateless Evaluators
By encapsulating all state in the frame, evaluators can be stateless. They receive their execution context as a parameter rather than storing it, enabling:
- Single evaluator instance for entire application lifetime
- Thread safety (if needed in the future)
- Cleaner testing with different contexts
- Better separation of concerns

## Usage Example

```c
// Create frame components
data_t *memory = ar__data__create_map();
data_t *context = ar__data__create_map();
data_t *message = ar__data__create_string("hello");

// Create frame
ar_frame_t *frame = ar_frame__create(memory, context, message);
if (!frame) {
    // Handle error
}

// Pass frame to evaluator
bool result = some_evaluator_function(evaluator, ast, frame);

// Clean up frame (but not the components)
ar_frame__destroy(frame);

// Components are still valid and owned by caller
ar__data__destroy(memory);
ar__data__destroy(context);
ar__data__destroy(message);
```

## Dependencies

- `ar_data`: For the data_t type used by all components
- `ar_heap`: For memory allocation tracking
- `ar_assert`: For debug assertions

## Testing

The module includes comprehensive tests covering:
- Basic creation and destruction
- NULL parameter rejection
- Accessor functions returning correct references
- Memory leak prevention

All tests follow TDD methodology and verify zero memory leaks.