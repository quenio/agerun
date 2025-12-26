#include "ar_file_delegate.h"
#include "ar_heap.h"
#include "ar_delegate.h"
#include <stdlib.h>

struct ar_file_delegate_s {
    ar_delegate_t *own_delegate;
    ar_log_t *ref_log;
    char *own_allowed_path;
};

ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    // Note: NULL log is acceptable - log module handles NULL gracefully
    if (!ref_allowed_path) {
        return NULL;
    }

    ar_file_delegate_t *own_file_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t), "file_delegate");
    if (!own_file_delegate) {
        return NULL;  // Handle malloc failure
    }

    own_file_delegate->ref_log = ref_log;
    own_file_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path, "file delegate allowed path");
    if (!own_file_delegate->own_allowed_path) {
        AR__HEAP__FREE(own_file_delegate);
        return NULL;  // Handle strdup failure and clean up delegate
    }

    // Create the base delegate wrapper
    own_file_delegate->own_delegate = ar_delegate__create(ref_log, "file");
    if (!own_file_delegate->own_delegate) {
        AR__HEAP__FREE(own_file_delegate->own_allowed_path);
        AR__HEAP__FREE(own_file_delegate);
        return NULL;
    }

    return own_file_delegate;
}

void ar_file_delegate__destroy(ar_file_delegate_t *own_file_delegate) {
    if (!own_file_delegate) {
        return;  // Safe NULL handling
    }

    if (own_file_delegate->own_delegate) {
        ar_delegate__destroy(own_file_delegate->own_delegate);
    }

    if (own_file_delegate->own_allowed_path) {
        AR__HEAP__FREE(own_file_delegate->own_allowed_path);
    }

    AR__HEAP__FREE(own_file_delegate);
}

const char* ar_file_delegate__get_type(const ar_file_delegate_t *ref_delegate) {
    (void)ref_delegate;  // Unused in minimal implementation
    return "file";  // Hardcoded - minimal implementation
}

