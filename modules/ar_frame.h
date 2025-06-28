/**
 * @file ar_frame.h
 * @brief Frame module for AgeRun evaluation context
 * 
 * This module provides the frame abstraction that encapsulates the complete
 * evaluation context needed by evaluators: memory, context, and message.
 * 
 * A frame represents the execution context for evaluating expressions and
 * instructions. It contains:
 * - memory: The agent's mutable state (variables)
 * - context: The agent's creation-time configuration (immutable)
 * - message: The current message being processed
 * 
 * All three components are required (non-NULL) to create a frame, ensuring
 * evaluators always have a complete context for evaluation.
 * 
 * Frames enable stateless evaluators by passing all necessary state as
 * parameters rather than storing it in evaluator instances.
 */

#ifndef AGERUN_FRAME_H
#define AGERUN_FRAME_H

#include "ar_data.h"

/**
 * Opaque frame structure.
 * Contains references to memory, context, and message for evaluation.
 */
typedef struct ar_frame_s ar_frame_t;

/**
 * Creates a new frame with the given memory, context, and message.
 * 
 * All three parameters are required and must be non-NULL. The frame
 * holds borrowed references to these objects - it does not take ownership.
 * 
 * @param mut_memory The agent's memory (mutable reference, required)
 * @param ref_context The agent's context (borrowed reference, required)
 * @param ref_message The message being processed (borrowed reference, required)
 * @return A new frame instance, or NULL if any parameter is NULL
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The frame does not take ownership of the parameters.
 */
ar_frame_t* ar_frame__create(
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
);

/**
 * Destroys a frame and releases its resources.
 * 
 * This does not destroy the memory, context, or message objects -
 * only the frame structure itself.
 * 
 * @param own_frame The frame to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the frame and destroys it.
 *       Safe to call with NULL.
 */
void ar_frame__destroy(ar_frame_t *own_frame);

/**
 * Gets the memory from the frame.
 * 
 * @param ref_frame The frame to query (borrowed reference)
 * @return The memory map (mutable reference), never NULL for valid frames
 * @note Ownership: Returns a mutable reference. The caller should not destroy it.
 */
data_t* ar_frame__get_memory(const ar_frame_t *ref_frame);

/**
 * Gets the context from the frame.
 * 
 * @param ref_frame The frame to query (borrowed reference)
 * @return The context data (borrowed reference), never NULL for valid frames
 * @note Ownership: Returns a borrowed reference. The caller should not destroy it.
 */
const data_t* ar_frame__get_context(const ar_frame_t *ref_frame);

/**
 * Gets the message from the frame.
 * 
 * @param ref_frame The frame to query (borrowed reference)
 * @return The message data (borrowed reference), never NULL for valid frames
 * @note Ownership: Returns a borrowed reference. The caller should not destroy it.
 */
const data_t* ar_frame__get_message(const ar_frame_t *ref_frame);

#endif /* AGERUN_FRAME_H */