#include "ar_file_delegate.h"
#include "ar_delegate.h"
#include "ar_heap.h"
#include "ar_io.h"
#include "ar_path.h"
#include "ar_log.h"
#include <stdio.h>
#include <string.h>

#define AR_FILE_DELEGATE__DEFAULT_MAX_FILE_SIZE (10 * 1024 * 1024)

struct ar_file_delegate_s {
    ar_delegate_t *own_delegate;
    ar_log_t *ref_log;
    char *own_allowed_path;
    size_t max_file_size;
};

static bool _is_valid_relative_path(const char *ref_path) {
    if (!ref_path || ref_path[0] == '\0') {
        return false;
    }
    if (ref_path[0] == '/') {
        return false;
    }

    ar_path_t *own_path = ar_path__create_file(ref_path);
    if (!own_path) {
        return false;
    }

    size_t ref_segment_count = ar_path__get_segment_count(own_path);
    for (size_t mut_index = 0; mut_index < ref_segment_count; ++mut_index) {
        const char *ref_segment = ar_path__get_segment(own_path, mut_index);
        if (!ref_segment || strcmp(ref_segment, "..") == 0) {
            ar_path__destroy(own_path);
            return false;
        }
    }

    ar_path__destroy(own_path);
    return true;
}

static char* _build_full_path(const char *ref_base_path, const char *ref_relative_path) {
    if (!ref_base_path || !ref_relative_path) {
        return NULL;
    }

    size_t ref_base_len = strlen(ref_base_path);
    size_t ref_relative_len = strlen(ref_relative_path);
    bool ref_needs_separator = (ref_base_len > 0 && ref_base_path[ref_base_len - 1] != '/');
    size_t ref_total_len = ref_base_len + (ref_needs_separator ? 1 : 0) + ref_relative_len;

    char *own_full_path = AR__HEAP__MALLOC(ref_total_len + 1, "file delegate full path");
    if (!own_full_path) {
        return NULL;
    }

    if (ref_base_len > 0) {
        memcpy(own_full_path, ref_base_path, ref_base_len);
    }
    if (ref_needs_separator) {
        own_full_path[ref_base_len] = '/';
    }
    memcpy(own_full_path + ref_base_len + (ref_needs_separator ? 1 : 0),
           ref_relative_path,
           ref_relative_len);
    own_full_path[ref_total_len] = '\0';

    return own_full_path;  // Ownership transferred to caller
}

static ar_data_t* _create_response(const char *ref_status) {
    ar_data_t *own_response = ar_data__create_map();
    if (!own_response) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_response, "status", ref_status)) {
        ar_data__destroy(own_response);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

static ar_data_t* _create_error_response(ar_log_t *ref_log, const char *ref_message) {
    ar_log__error(ref_log, ref_message);

    ar_data_t *own_response = _create_response("error");
    if (!own_response) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_response, "message", ref_message)) {
        ar_data__destroy(own_response);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

static ar_data_t* _create_success_response(void) {
    ar_data_t *own_response = _create_response("success");
    if (!own_response) {
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

static ar_data_t* _create_success_response_with_content(const char *ref_content) {
    ar_data_t *own_response = _create_response("success");
    if (!own_response) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_response, "content", ref_content)) {
        ar_data__destroy(own_response);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

static ar_data_t* _handle_read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!_is_valid_relative_path(ref_path)) {
        return _create_error_response(mut_delegate->ref_log, "Invalid path");
    }

    char *own_full_path = _build_full_path(mut_delegate->own_allowed_path, ref_path);
    if (!own_full_path) {
        return _create_error_response(mut_delegate->ref_log, "Failed to build path");
    }

    FILE *mut_file = NULL;
    ar_file_result_t ref_open_result = ar_io__open_file(own_full_path, "r", &mut_file);
    if (ref_open_result != AR_FILE_RESULT__SUCCESS) {
        ar_data_t *own_response = _create_error_response(
            mut_delegate->ref_log,
            ar_io__error_message(ref_open_result));
        AR__HEAP__FREE(own_full_path);
        return own_response;  // Ownership transferred to caller
    }

    if (fseek(mut_file, 0, SEEK_END) != 0) {
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "Failed to read file size");
    }

    long ref_file_size = ftell(mut_file);
    if (ref_file_size < 0) {
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "Failed to read file size");
    }

    if (ref_file_size > (long)mut_delegate->max_file_size) {
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "File too large");
    }

    if (fseek(mut_file, 0, SEEK_SET) != 0) {
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "Failed to read file");
    }

    size_t ref_buffer_size = (size_t)ref_file_size;
    char *own_buffer = AR__HEAP__MALLOC(ref_buffer_size + 1, "file delegate read buffer");
    if (!own_buffer) {
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "Failed to allocate file buffer");
    }

    size_t ref_bytes_read = fread(own_buffer, 1, ref_buffer_size, mut_file);
    if (ref_bytes_read != ref_buffer_size) {
        AR__HEAP__FREE(own_buffer);
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "Failed to read file");
    }

    own_buffer[ref_buffer_size] = '\0';

    ar_data_t *own_response = _create_success_response_with_content(own_buffer);
    AR__HEAP__FREE(own_buffer);
    ar_io__close_file(mut_file, own_full_path);
    AR__HEAP__FREE(own_full_path);
    return own_response;  // Ownership transferred to caller
}

static ar_data_t* _handle_write(ar_file_delegate_t *mut_delegate,
                               const char *ref_path,
                               const char *ref_content) {
    if (!_is_valid_relative_path(ref_path)) {
        return _create_error_response(mut_delegate->ref_log, "Invalid path");
    }
    if (!ref_content) {
        return _create_error_response(mut_delegate->ref_log, "Missing content");
    }

    char *own_full_path = _build_full_path(mut_delegate->own_allowed_path, ref_path);
    if (!own_full_path) {
        return _create_error_response(mut_delegate->ref_log, "Failed to build path");
    }

    FILE *mut_file = NULL;
    ar_file_result_t ref_open_result = ar_io__open_file(own_full_path, "w", &mut_file);
    if (ref_open_result != AR_FILE_RESULT__SUCCESS) {
        ar_data_t *own_response = _create_error_response(
            mut_delegate->ref_log,
            ar_io__error_message(ref_open_result));
        AR__HEAP__FREE(own_full_path);
        return own_response;  // Ownership transferred to caller
    }

    size_t ref_content_len = strlen(ref_content);
    size_t ref_bytes_written = fwrite(ref_content, 1, ref_content_len, mut_file);
    if (ref_bytes_written != ref_content_len) {
        ar_io__close_file(mut_file, own_full_path);
        AR__HEAP__FREE(own_full_path);
        return _create_error_response(mut_delegate->ref_log, "Failed to write file");
    }

    ar_io__close_file(mut_file, own_full_path);
    AR__HEAP__FREE(own_full_path);
    return _create_success_response();  // Ownership transferred to caller
}

ar_file_delegate_t* ar_file_delegate__create(
    ar_log_t *ref_log,
    const char *ref_allowed_path,
    size_t max_file_size) {
    // Note: NULL log is acceptable - log module handles NULL gracefully
    if (!ref_allowed_path) {
        return NULL;
    }

    ar_file_delegate_t *own_file_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t), "file_delegate");
    if (!own_file_delegate) {
        return NULL;  // Handle malloc failure
    }

    own_file_delegate->ref_log = ref_log;
    own_file_delegate->max_file_size = (max_file_size > 0)
        ? max_file_size
        : AR_FILE_DELEGATE__DEFAULT_MAX_FILE_SIZE;
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

ar_data_t* ar_file_delegate__handle_message(
    ar_file_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id) {
    (void)sender_id;

    if (!mut_delegate || !ref_message) {
        return _create_error_response(NULL, "Invalid message");
    }

    if (ar_data__get_type(ref_message) != AR_DATA_TYPE__MAP) {
        return _create_error_response(mut_delegate->ref_log, "Invalid message");
    }

    const char *ref_action = ar_data__get_map_string(ref_message, "action");
    const char *ref_path = ar_data__get_map_string(ref_message, "path");
    if (!ref_action || !ref_path) {
        return _create_error_response(mut_delegate->ref_log, "Invalid message");
    }

    if (strcmp(ref_action, "read") == 0) {
        return _handle_read(mut_delegate, ref_path);  // Ownership transferred to caller
    }

    if (strcmp(ref_action, "write") == 0) {
        const char *ref_content = ar_data__get_map_string(ref_message, "content");
        return _handle_write(mut_delegate, ref_path, ref_content);  // Ownership transferred to caller
    }

    return _create_error_response(mut_delegate->ref_log, "Unknown action");
}
