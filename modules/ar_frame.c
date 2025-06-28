/**
 * @file ar_frame.c
 * @brief Implementation of the frame module
 */

#include "ar_frame.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include <stddef.h>

/**
 * Internal structure for frame
 */
struct ar_frame_s {
    data_t *mut_memory;        /* Agent's memory (mutable reference) */
    data_t *ref_context;       /* Agent's context (borrowed reference) */
    data_t *ref_message;       /* Current message (borrowed reference) */
};

/**
 * Creates a new frame with the given memory, context, and message
 */
ar_frame_t* ar_frame__create(
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message)
{
    // All three parameters are required
    if (!mut_memory || !ref_context || !ref_message) {
        return NULL;
    }
    
    // Allocate frame structure
    ar_frame_t *frame = AR__HEAP__MALLOC(sizeof(ar_frame_t), "frame");
    if (!frame) {
        return NULL;
    }
    
    // Store borrowed references
    frame->mut_memory = mut_memory;
    frame->ref_context = ref_context;
    frame->ref_message = ref_message;
    
    return frame;
}

/**
 * Destroys a frame
 */
void ar_frame__destroy(ar_frame_t *own_frame)
{
    if (own_frame) {
        // Frame doesn't own the memory, context, or message
        // Just free the frame structure itself
        AR__HEAP__FREE(own_frame);
    }
}

/**
 * Gets the memory from the frame
 */
data_t* ar_frame__get_memory(const ar_frame_t *ref_frame)
{
    AR_ASSERT(ref_frame != NULL, "Frame cannot be NULL");
    AR_ASSERT(ref_frame->mut_memory != NULL, "Frame memory should never be NULL");
    return ref_frame->mut_memory;
}

/**
 * Gets the context from the frame
 */
const data_t* ar_frame__get_context(const ar_frame_t *ref_frame)
{
    AR_ASSERT(ref_frame != NULL, "Frame cannot be NULL");
    AR_ASSERT(ref_frame->ref_context != NULL, "Frame context should never be NULL");
    return ref_frame->ref_context;
}

/**
 * Gets the message from the frame
 */
const data_t* ar_frame__get_message(const ar_frame_t *ref_frame)
{
    AR_ASSERT(ref_frame != NULL, "Frame cannot be NULL");
    AR_ASSERT(ref_frame->ref_message != NULL, "Frame message should never be NULL");
    return ref_frame->ref_message;
}