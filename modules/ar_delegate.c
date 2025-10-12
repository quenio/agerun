#include "ar_delegate.h"
#include "ar_heap.h"
#include <stdbool.h>
#include <stdlib.h>

struct ar_delegate_s {
    ar_log_t *ref_log;
    const char *type;
};

ar_delegate_t* ar_delegate__create(ar_log_t *ref_log, const char *type) {
    ar_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_delegate_t), "delegate");
    if (!own_delegate) {
        return NULL;
    }

    own_delegate->ref_log = ref_log;
    own_delegate->type = type;

    return own_delegate;
}

void ar_delegate__destroy(ar_delegate_t *own_delegate) {
    if (!own_delegate) {
        return;
    }

    AR__HEAP__FREE(own_delegate);
}

ar_log_t* ar_delegate__get_log(const ar_delegate_t *ref_delegate) {
    if (!ref_delegate) {
        return NULL;
    }

    return ref_delegate->ref_log;
}

const char* ar_delegate__get_type(const ar_delegate_t *ref_delegate) {
    if (!ref_delegate) {
        return NULL;
    }

    return ref_delegate->type;
}

bool ar_delegate__handle_message(ar_delegate_t *ref_delegate, ar_data_t *ref_message, int64_t sender_id) {
    (void)ref_message;
    (void)sender_id;

    if (!ref_delegate) {
        return false;
    }

    // No handler configured yet, return false
    return false;
}
