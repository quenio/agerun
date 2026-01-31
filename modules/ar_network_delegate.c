#include "ar_network_delegate.h"
#include "ar_delegate.h"
#include "ar_heap.h"
#include "ar_log.h"
#include <string.h>

#define AR_NETWORK_DELEGATE__DEFAULT_MAX_RESPONSE_SIZE (1024 * 1024)
#define AR_NETWORK_DELEGATE__DEFAULT_TIMEOUT_SECONDS 30

struct ar_network_delegate_s {
    ar_delegate_t *own_delegate;
    ar_log_t *ref_log;
    char **own_whitelist;
    size_t whitelist_count;
    size_t max_response_size;
    int timeout_seconds;
};

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

static ar_data_t* _create_success_response(const char *ref_content) {
    ar_data_t *own_response = _create_response("success");
    if (!own_response) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_response, "content", ref_content)) {
        ar_data__destroy(own_response);
        return NULL;
    }

    if (!ar_data__set_map_integer(own_response, "stubbed", 1)) {
        ar_data__destroy(own_response);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

static bool _is_url_whitelisted(const ar_network_delegate_t *ref_delegate, const char *ref_url) {
    if (!ref_delegate || !ref_url) {
        return false;
    }

    if (ref_delegate->whitelist_count == 0) {
        return true;
    }

    for (size_t mut_index = 0; mut_index < ref_delegate->whitelist_count; ++mut_index) {
        const char *ref_prefix = ref_delegate->own_whitelist[mut_index];
        if (!ref_prefix) {
            continue;
        }

        size_t ref_prefix_len = strlen(ref_prefix);
        if (strncmp(ref_url, ref_prefix, ref_prefix_len) == 0) {
            return true;
        }
    }

    return false;
}

static ar_data_t* _handle_stub_request(ar_network_delegate_t *mut_delegate) {
    const char *ref_content = "stub";
    size_t ref_content_len = strlen(ref_content);
    if (ref_content_len > mut_delegate->max_response_size) {
        return _create_error_response(mut_delegate->ref_log, "Response too large");
    }

    return _create_success_response(ref_content);  // Ownership transferred to caller
}

ar_network_delegate_t* ar_network_delegate__create(
    ar_log_t *ref_log,
    const char **ref_whitelist,
    size_t ref_whitelist_count,
    size_t ref_max_response_size,
    int ref_timeout_seconds) {
    ar_network_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_network_delegate_t), "network delegate");
    if (!own_delegate) {
        return NULL;
    }

    own_delegate->ref_log = ref_log;
    own_delegate->own_whitelist = NULL;
    own_delegate->whitelist_count = ref_whitelist_count;
    own_delegate->max_response_size = (ref_max_response_size > 0)
        ? ref_max_response_size
        : AR_NETWORK_DELEGATE__DEFAULT_MAX_RESPONSE_SIZE;
    own_delegate->timeout_seconds = (ref_timeout_seconds > 0)
        ? ref_timeout_seconds
        : AR_NETWORK_DELEGATE__DEFAULT_TIMEOUT_SECONDS;

    if (ref_whitelist_count > 0) {
        own_delegate->own_whitelist = AR__HEAP__MALLOC(sizeof(char*) * ref_whitelist_count,
                                                       "network delegate whitelist");
        if (!own_delegate->own_whitelist) {
            AR__HEAP__FREE(own_delegate);
            return NULL;
        }

        for (size_t mut_index = 0; mut_index < ref_whitelist_count; ++mut_index) {
            const char *ref_entry = ref_whitelist[mut_index];
            own_delegate->own_whitelist[mut_index] = NULL;
            if (!ref_entry) {
                continue;
            }

            own_delegate->own_whitelist[mut_index] = AR__HEAP__STRDUP(ref_entry,
                                                                     "network delegate whitelist entry");
            if (!own_delegate->own_whitelist[mut_index]) {
                for (size_t mut_cleanup = 0; mut_cleanup <= mut_index; ++mut_cleanup) {
                    if (own_delegate->own_whitelist[mut_cleanup]) {
                        AR__HEAP__FREE(own_delegate->own_whitelist[mut_cleanup]);
                    }
                }
                AR__HEAP__FREE(own_delegate->own_whitelist);
                AR__HEAP__FREE(own_delegate);
                return NULL;
            }
        }
    }

    own_delegate->own_delegate = ar_delegate__create(ref_log, "network");
    if (!own_delegate->own_delegate) {
        if (own_delegate->own_whitelist) {
            for (size_t mut_index = 0; mut_index < own_delegate->whitelist_count; ++mut_index) {
                if (own_delegate->own_whitelist[mut_index]) {
                    AR__HEAP__FREE(own_delegate->own_whitelist[mut_index]);
                }
            }
            AR__HEAP__FREE(own_delegate->own_whitelist);
        }
        AR__HEAP__FREE(own_delegate);
        return NULL;
    }

    return own_delegate;
}

void ar_network_delegate__destroy(ar_network_delegate_t *own_delegate) {
    if (!own_delegate) {
        return;
    }

    if (own_delegate->own_delegate) {
        ar_delegate__destroy(own_delegate->own_delegate);
    }

    if (own_delegate->own_whitelist) {
        for (size_t mut_index = 0; mut_index < own_delegate->whitelist_count; ++mut_index) {
            if (own_delegate->own_whitelist[mut_index]) {
                AR__HEAP__FREE(own_delegate->own_whitelist[mut_index]);
            }
        }
        AR__HEAP__FREE(own_delegate->own_whitelist);
    }

    AR__HEAP__FREE(own_delegate);
}

const char* ar_network_delegate__get_type(const ar_network_delegate_t *ref_delegate) {
    (void)ref_delegate;
    return "network";
}

ar_data_t* ar_network_delegate__handle_message(
    ar_network_delegate_t *mut_delegate,
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
    const char *ref_url = ar_data__get_map_string(ref_message, "url");
    if (!ref_action || !ref_url) {
        return _create_error_response(mut_delegate->ref_log, "Invalid message");
    }

    if (!_is_url_whitelisted(mut_delegate, ref_url)) {
        return _create_error_response(mut_delegate->ref_log, "URL not whitelisted");
    }

    if (strcmp(ref_action, "GET") == 0) {
        return _handle_stub_request(mut_delegate);  // Ownership transferred to caller
    }

    if (strcmp(ref_action, "POST") == 0) {
        const char *ref_body = ar_data__get_map_string(ref_message, "body");
        if (!ref_body) {
            return _create_error_response(mut_delegate->ref_log, "Missing body");
        }
        return _handle_stub_request(mut_delegate);  // Ownership transferred to caller
    }

    return _create_error_response(mut_delegate->ref_log, "Unknown action");
}
