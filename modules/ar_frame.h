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
    ar_data_t *mut_memory,
    const ar_data_t *ref_context,
    const ar_data_t *ref_message
);

/**
 * Rebinds an existing frame to a new execution context.
 *
 * @param mut_frame The frame to update (mutable reference)
 * @param mut_memory The memory map containing variables (mutable reference)
 * @param ref_context The context map with additional data (const reference)
 * @param ref_message The message being processed (const reference)
 * @return true if successful, false if any parameter is NULL
 */
bool ar_frame__reset(
    ar_frame_t *mut_frame,
    ar_data_t *mut_memory,
    const ar_data_t *ref_context,
    const ar_data_t *ref_message
);

/**
 * Set the current owner token for the frame message
 * @param mut_frame The frame to update
 * @param ref_message_owner The current owner token for the message (borrowed reference)
 */
void ar_frame__set_message_owner(ar_frame_t *mut_frame, const void *ref_message_owner);

/**
 * Set the current executing agent ID for the frame.
 * @param mut_frame The frame to update
 * @param agent_id The currently executing agent ID
 */
void ar_frame__set_current_agent_id(ar_frame_t *mut_frame, int64_t agent_id);

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
ar_data_t* ar_frame__get_memory(const ar_frame_t *ref_frame);

/**
 * Gets the context map from a frame
 * @param ref_frame The frame to query
 * @return The context map (const reference)
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const ar_data_t* ar_frame__get_context(const ar_frame_t *ref_frame);

/**
 * Gets the message from a frame
 * @param ref_frame The frame to query
 * @return The message (const reference)
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const ar_data_t* ar_frame__get_message(const ar_frame_t *ref_frame);

/**
 * Gets the current owner token for the frame message
 * @param ref_frame The frame to query
 * @return The current message owner token, or NULL if none is set
 */
const void* ar_frame__get_message_owner(const ar_frame_t *ref_frame);

/**
 * Gets the current executing agent ID for the frame.
 * @param ref_frame The frame to query
 * @return The current agent ID, or 0 if unavailable
 */
int64_t ar_frame__get_current_agent_id(const ar_frame_t *ref_frame);

#endif /* AGERUN_FRAME_H */