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
    ar_frame_t *own_frame;

    if (!mut_memory || !ref_context || !ref_message) {
        return NULL;
    }
    
    own_frame = AR__HEAP__MALLOC(sizeof(ar_frame_t), "frame");
    if (!own_frame) {
        return NULL;
    }

    if (!ar_frame__reset(own_frame, mut_memory, ref_context, ref_message)) {
        AR__HEAP__FREE(own_frame);
        return NULL;
    }
    
    return own_frame;
    // Ownership transferred to caller
}

bool ar_frame__reset(
    ar_frame_t *mut_frame,
    ar_data_t *mut_memory,
    const ar_data_t *ref_context,
    const ar_data_t *ref_message
) {
    if (!mut_frame || !mut_memory || !ref_context || !ref_message) {
        return false;
    }

    mut_frame->mut_memory = mut_memory;
    mut_frame->ref_context = ref_context;
    mut_frame->ref_message = ref_message;
    return true;
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