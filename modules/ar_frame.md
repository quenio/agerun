# Frame Module

## Overview

The frame module provides a simple data structure that bundles the execution context (memory, context, message) for stateless evaluators. Frames are lightweight containers that hold references to the three key pieces of data needed during instruction evaluation.

## Purpose

The frame abstraction enables:
- Stateless evaluator functions that receive all context via a single parameter
- Clear separation between execution state and evaluation logic
- Simplified parameter passing throughout the evaluation chain
- Consistent access patterns for memory, context, and message data

## Key Design Principles

1. **Reference-Only**: Frames do not own any data - they only hold references
2. **Required Fields**: All three fields (memory, context, message) are required - no NULL values allowed
3. **Const-Correctness**: Context and message are const references (read-only), memory is mutable
4. **Minimal Interface**: Only essential operations are exposed (create, destroy, getters)

## Usage Example

```c
// Create execution context
ar_data_t *own_memory = ar_data__create_map();
ar_data_t *own_context = ar_data__create_map();
ar_data_t *own_message = ar_data__create_map();

// Create frame bundling the context
ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);

// Access data through frame
ar_data_t *mut_memory = ar_frame__get_memory(own_frame);
const ar_data_t *ref_context = ar_frame__get_context(own_frame);
const ar_data_t *ref_message = ar_frame__get_message(own_frame);

// Use frame in evaluators...

// Clean up
ar_frame__destroy(own_frame);
ar_data__destroy(own_memory);
ar_data__destroy(own_context);
ar_data__destroy(own_message);
```

## Functions

### ar_frame__create
Creates a new frame with the given execution context. All parameters are required (cannot be NULL).

### ar_frame__destroy
Destroys a frame and frees its structure. Does not destroy the referenced data.

### ar_frame__get_memory
Returns the mutable memory map reference from the frame.

### ar_frame__get_context
Returns the const context map reference from the frame.

### ar_frame__get_message
Returns the const message reference from the frame.

## Integration Notes

The frame module will be used throughout the evaluator chain:
- Expression evaluators will be updated to accept frames instead of separate parameters
- Instruction evaluators will pass frames to expression evaluators
- Method evaluators will create frames and pass them to instruction evaluators

This creates a consistent execution model where all evaluation functions receive their full context through a single frame parameter.