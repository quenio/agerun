/**
 * @file ar_frame.c
 * @brief Implementation of the frame module
 */

#include "ar_frame.h"
#include "ar_heap.h"
#include "ar_data.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Frame structure definition
 */
struct ar_frame_s {
    ar_data_t *mut_memory;         // Mutable reference to memory
    const ar_data_t *ref_context;  // Const reference to context
    const ar_data_t *ref_message;  // Const reference to message
};

ar_frame_t* ar_frame__create(
    ar_data_t *mut_memory,
    const ar_data_t *ref_context,
    const ar_data_t *ref_message
) {
    // Validate parameters - all fields are required
    if (!mut_memory || !ref_context || !ref_message) {
        return NULL;
    }
    
    // DEBUG: Log frame creation with message type
    fprintf(stderr, "DEBUG [FRAME_CREATE]: Creating frame with message type=%d", ar_data__get_type(ref_message));
    if (ar_data__get_type(ref_message) == AR_DATA_TYPE__INTEGER) {
        fprintf(stderr, ", value=%lld", (long long)ar_data__get_integer(ref_message));
    }
    fprintf(stderr, "\n");
    
    // Allocate frame structure
    ar_frame_t *own_frame = AR__HEAP__MALLOC(sizeof(ar_frame_t), "frame");
    if (!own_frame) {
        return NULL;
    }
    
    // Store references
    own_frame->mut_memory = mut_memory;
    own_frame->ref_context = ref_context;
    own_frame->ref_message = ref_message;
    
    return own_frame;
    // Ownership transferred to caller
}

void ar_frame__destroy(ar_frame_t *own_frame) {
    if (!own_frame) {
        return;
    }
    
    // Frame doesn't own the data, just free the structure
    AR__HEAP__FREE(own_frame);
}

ar_data_t* ar_frame__get_memory(const ar_frame_t *ref_frame) {
    if (!ref_frame) {
        return NULL;
    }
    return ref_frame->mut_memory;
}

const ar_data_t* ar_frame__get_context(const ar_frame_t *ref_frame) {
    if (!ref_frame) {
        return NULL;
    }
    return ref_frame->ref_context;
}

const ar_data_t* ar_frame__get_message(const ar_frame_t *ref_frame) {
    if (!ref_frame) {
        return NULL;
    }
    return ref_frame->ref_message;
}