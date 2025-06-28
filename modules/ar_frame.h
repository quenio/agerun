/**
 * @file ar_frame.h
 * @brief Frame module for bundling execution context
 * 
 * The frame module provides a simple data structure that bundles
 * the execution context (memory, context, message) for stateless
 * evaluators. Frames do not own any data - they only hold references.
 * 
 * All fields are required (no NULL values allowed).
 */

#ifndef AGERUN_FRAME_H
#define AGERUN_FRAME_H

#include "ar_data.h"

/**
 * Opaque type for execution frame
 */
typedef struct ar_frame_s ar_frame_t;

/**
 * Creates a new frame with the given execution context
 * @param mut_memory The memory map containing variables (mutable reference)
 * @param ref_context The context map with additional data (const reference)
 * @param ref_message The message being processed (const reference)
 * @return A new frame instance, or NULL if any parameter is NULL
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The frame does not take ownership of any parameters.
 */
ar_frame_t* ar_frame__create(
    data_t *mut_memory,
    const data_t *ref_context,
    const data_t *ref_message
);

/**
 * Destroys a frame and frees its resources
 * @param own_frame The frame to destroy
 * @note Ownership: Takes ownership and destroys the frame.
 *       Does not destroy the referenced data.
 */
void ar_frame__destroy(ar_frame_t *own_frame);

/**
 * Gets the memory map from a frame
 * @param ref_frame The frame to query
 * @return The memory map (mutable reference)
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
data_t* ar_frame__get_memory(const ar_frame_t *ref_frame);

/**
 * Gets the context map from a frame
 * @param ref_frame The frame to query
 * @return The context map (const reference)
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const data_t* ar_frame__get_context(const ar_frame_t *ref_frame);

/**
 * Gets the message from a frame
 * @param ref_frame The frame to query
 * @return The message (const reference)
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const data_t* ar_frame__get_message(const ar_frame_t *ref_frame);

#endif /* AGERUN_FRAME_H */