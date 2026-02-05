#include "ar_log_delegate.h"
#include "ar_delegate.h"
#include "ar_heap.h"
#include "ar_log.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define AR_LOG_DELEGATE__DEFAULT_MIN_LEVEL 0

typedef enum ar_log_delegate_level_e {
    AR_LOG_DELEGATE_LEVEL__INFO = 0,
    AR_LOG_DELEGATE_LEVEL__WARNING = 1,
    AR_LOG_DELEGATE_LEVEL__ERROR = 2
} ar_log_delegate_level_t;

struct ar_log_delegate_s {
    ar_delegate_t *own_delegate;
    ar_log_t *ref_log;
    ar_log_delegate_level_t min_level;
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

static ar_data_t* _create_success_response(void) {
    ar_data_t *own_response = _create_response("success");
    if (!own_response) {
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

static bool _parse_level(const char *ref_level, ar_log_delegate_level_t *mut_level) {
    if (!ref_level || !mut_level) {
        return false;
    }

    if (strcmp(ref_level, "info") == 0) {
        *mut_level = AR_LOG_DELEGATE_LEVEL__INFO;
        return true;
    }

    if (strcmp(ref_level, "warning") == 0) {
        *mut_level = AR_LOG_DELEGATE_LEVEL__WARNING;
        return true;
    }

    if (strcmp(ref_level, "error") == 0) {
        *mut_level = AR_LOG_DELEGATE_LEVEL__ERROR;
        return true;
    }

    return false;
}

static const char* _level_to_string(ar_log_delegate_level_t level) {
    switch (level) {
        case AR_LOG_DELEGATE_LEVEL__INFO:
            return "info";
        case AR_LOG_DELEGATE_LEVEL__WARNING:
            return "warning";
        case AR_LOG_DELEGATE_LEVEL__ERROR:
            return "error";
        default:
            return "info";
    }
}

static void _format_timestamp(char *mut_buffer, size_t buffer_size) {
    if (!mut_buffer || buffer_size == 0) {
        return;
    }

    time_t ref_raw_time = time(NULL);
    struct tm *ref_time = gmtime(&ref_raw_time);
    if (!ref_time) {
        strncpy(mut_buffer, "unknown", buffer_size - 1);
        mut_buffer[buffer_size - 1] = '\0';
        return;
    }

    strftime(mut_buffer, buffer_size, "%Y-%m-%dT%H:%M:%SZ", ref_time);
}

static char* _format_log_message(const char *ref_level,
                                 const char *ref_message,
                                 int64_t agent_id) {
    char mut_timestamp[32] = {0};
    _format_timestamp(mut_timestamp, sizeof(mut_timestamp));

    size_t ref_level_len = strlen(ref_level);
    size_t ref_message_len = strlen(ref_message);
    size_t ref_timestamp_len = strlen(mut_timestamp);
    size_t ref_total_len = ref_level_len + ref_message_len + ref_timestamp_len + 64;

    char *own_formatted = AR__HEAP__MALLOC(ref_total_len, "log delegate message");
    if (!own_formatted) {
        return NULL;
    }

    snprintf(own_formatted,
             ref_total_len,
             "%s level=%s agent=%" PRId64 " message=%s",
             mut_timestamp,
             ref_level,
             agent_id,
             ref_message);

    return own_formatted;  // Ownership transferred to caller
}

ar_log_delegate_t* ar_log_delegate__create(ar_log_t *ref_log, const char *ref_min_level) {
    ar_log_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_log_delegate_t), "log delegate");
    if (!own_delegate) {
        return NULL;
    }

    own_delegate->ref_log = ref_log;
    own_delegate->min_level = AR_LOG_DELEGATE__DEFAULT_MIN_LEVEL;

    if (ref_min_level) {
        ar_log_delegate_level_t mut_parsed_level = AR_LOG_DELEGATE__DEFAULT_MIN_LEVEL;
        if (_parse_level(ref_min_level, &mut_parsed_level)) {
            own_delegate->min_level = mut_parsed_level;
        }
    }

    own_delegate->own_delegate = ar_delegate__create(ref_log, "log");
    if (!own_delegate->own_delegate) {
        AR__HEAP__FREE(own_delegate);
        return NULL;
    }

    return own_delegate;
}

void ar_log_delegate__destroy(ar_log_delegate_t *own_delegate) {
    if (!own_delegate) {
        return;
    }

    if (own_delegate->own_delegate) {
        ar_delegate__destroy(own_delegate->own_delegate);
    }

    AR__HEAP__FREE(own_delegate);
}

const char* ar_log_delegate__get_type(const ar_log_delegate_t *ref_delegate) {
    (void)ref_delegate;
    return "log";
}

ar_data_t* ar_log_delegate__handle_message(
    ar_log_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id) {
    if (!mut_delegate || !ref_message) {
        return _create_error_response(NULL, "Invalid message");
    }

    if (ar_data__get_type(ref_message) != AR_DATA_TYPE__MAP) {
        return _create_error_response(mut_delegate->ref_log, "Invalid message");
    }

    const char *ref_level = ar_data__get_map_string(ref_message, "level");
    const char *ref_text = ar_data__get_map_string(ref_message, "message");
    ar_data_t *ref_agent_id_data = ar_data__get_map_data(ref_message, "agent_id");

    if (!ref_level || !ref_text) {
        return _create_error_response(mut_delegate->ref_log, "Invalid message");
    }

    if (ref_agent_id_data && ar_data__get_type(ref_agent_id_data) != AR_DATA_TYPE__INTEGER) {
        return _create_error_response(mut_delegate->ref_log, "Invalid agent_id");
    }

    if (ref_agent_id_data) {
        int64_t ref_message_agent_id = ar_data__get_integer(ref_agent_id_data);
        if (ref_message_agent_id != sender_id) {
            return _create_error_response(mut_delegate->ref_log, "agent_id does not match sender_id");
        }
    }

    ar_log_delegate_level_t mut_parsed_level = AR_LOG_DELEGATE__DEFAULT_MIN_LEVEL;
    if (!_parse_level(ref_level, &mut_parsed_level)) {
        return _create_error_response(mut_delegate->ref_log, "Invalid log level");
    }

    if (mut_parsed_level < mut_delegate->min_level) {
        return _create_error_response(mut_delegate->ref_log, "Log level below minimum");
    }

    const char *ref_level_string = _level_to_string(mut_parsed_level);
    char *own_log_message = _format_log_message(ref_level_string, ref_text, sender_id);
    if (!own_log_message) {
        return _create_error_response(mut_delegate->ref_log, "Failed to format log message");
    }

    if (mut_parsed_level == AR_LOG_DELEGATE_LEVEL__INFO) {
        ar_log__info(mut_delegate->ref_log, own_log_message);
    } else if (mut_parsed_level == AR_LOG_DELEGATE_LEVEL__WARNING) {
        ar_log__warning(mut_delegate->ref_log, own_log_message);
    } else {
        ar_log__error(mut_delegate->ref_log, own_log_message);
    }

    AR__HEAP__FREE(own_log_message);
    return _create_success_response();  // Ownership transferred to caller
}
