#include "ar_shell_session.h"
#include "ar_heap.h"
#include <string.h>

static const char *AR_SHELL_SESSION__ACTION__RETURN_LOADED_VALUE =
    "ar_shell_session__return_loaded_value";
static const char *AR_SHELL_SESSION__ACTION__REPORT_OPERATION_FAILURE =
    "ar_shell_session__report_operation_failure";

static const char* _memory_relative_path(const char *ref_path);

struct ar_shell_session_s {
    int64_t session_id;
    ar_shell_mode_t mode;
    int64_t agent_id;
    bool is_active;
    ar_data_t *own_memory;
};

static const char* _memory_relative_path(const char *ref_path) {
    if (!ref_path) {
        return NULL;
    }

    if (strcmp(ref_path, "memory") == 0) {
        return "";
    }

    if (strncmp(ref_path, "memory.", strlen("memory.")) != 0) {
        return NULL;
    }

    return ref_path + strlen("memory.");
}

ar_shell_session_t* ar_shell_session__create(int64_t session_id, ar_shell_mode_t mode) {
    ar_shell_session_t *own_session = AR__HEAP__MALLOC(sizeof(ar_shell_session_t), "shell session");
    if (!own_session) {
        return NULL;
    }

    own_session->session_id = session_id;
    own_session->mode = mode;
    own_session->agent_id = 0;
    own_session->is_active = false;
    own_session->own_memory = ar_data__create_map();
    if (!own_session->own_memory) {
        AR__HEAP__FREE(own_session);
        return NULL;
    }

    return own_session;
}

void ar_shell_session__destroy(ar_shell_session_t *own_session) {
    if (!own_session) {
        return;
    }

    if (own_session->own_memory) {
        ar_data__destroy(own_session->own_memory);
    }

    AR__HEAP__FREE(own_session);
}

int64_t ar_shell_session__get_session_id(const ar_shell_session_t *ref_session) {
    if (!ref_session) {
        return 0;
    }

    return ref_session->session_id;
}

ar_shell_mode_t ar_shell_session__get_mode(const ar_shell_session_t *ref_session) {
    if (!ref_session) {
        return AR_SHELL_MODE__NORMAL;
    }

    return ref_session->mode;
}

int64_t ar_shell_session__get_agent_id(const ar_shell_session_t *ref_session) {
    if (!ref_session) {
        return 0;
    }

    return ref_session->agent_id;
}

bool ar_shell_session__activate(ar_shell_session_t *mut_session, int64_t agent_id) {
    if (!mut_session || agent_id <= 0) {
        return false;
    }

    mut_session->agent_id = agent_id;
    mut_session->is_active = true;
    return true;
}

bool ar_shell_session__store_value(
    ar_shell_session_t *mut_session,
    const char *ref_path,
    ar_data_t *own_value) {
    if (!mut_session || !mut_session->own_memory || !ref_path || !own_value) {
        return false;
    }

    return ar_data__set_map_data_if_root_matched(mut_session->own_memory, "memory", ref_path, own_value);
}

ar_data_t* ar_shell_session__return_loaded_value(int request_id, ar_data_t *own_value) {
    ar_data_t *own_response;

    if (!own_value) {
        return NULL;
    }

    own_response = ar_data__create_map();
    if (!own_response) {
        ar_data__destroy(own_value);
        return NULL;
    }

    if (!ar_data__set_map_string(
            own_response,
            "action",
            AR_SHELL_SESSION__ACTION__RETURN_LOADED_VALUE) ||
        !ar_data__set_map_integer(own_response, "request_id", request_id) ||
        !ar_data__set_map_data(own_response, "value", own_value)) {
        ar_data__destroy(own_response);
        ar_data__destroy(own_value);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

ar_data_t* ar_shell_session__report_operation_failure(int request_id, const char *ref_reason) {
    ar_data_t *own_response;

    if (!ref_reason) {
        return NULL;
    }

    own_response = ar_data__create_map();
    if (!own_response) {
        return NULL;
    }

    if (!ar_data__set_map_string(
            own_response,
            "action",
            AR_SHELL_SESSION__ACTION__REPORT_OPERATION_FAILURE) ||
        !ar_data__set_map_integer(own_response, "request_id", request_id) ||
        !ar_data__set_map_string(own_response, "reason", ref_reason)) {
        ar_data__destroy(own_response);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

ar_data_t* ar_shell_session__load_value(
    const ar_shell_session_t *ref_session,
    const char *ref_path,
    int request_id) {
    const char *ref_relative_path;
    ar_data_t *ref_stored_value;
    ar_data_t *own_loaded_value;
    ar_data_t *own_response;

    if (!ref_session || !ref_session->own_memory || !ref_path) {
        return NULL;
    }

    ref_relative_path = _memory_relative_path(ref_path);
    if (!ref_relative_path) {
        return ar_shell_session__report_operation_failure(request_id, "Invalid shell session path");
    }

    if (ref_relative_path[0] == '\0') {
        own_loaded_value = ar_data__shallow_copy(ref_session->own_memory);
    } else {
        ref_stored_value = ar_data__get_map_data(ref_session->own_memory, ref_relative_path);
        if (!ref_stored_value) {
            return ar_shell_session__report_operation_failure(request_id, "Shell session path not found");
        }
        own_loaded_value = ar_data__shallow_copy(ref_stored_value);
    }

    if (!own_loaded_value) {
        return ar_shell_session__report_operation_failure(request_id, "Failed to copy shell session value");
    }

    own_response = ar_shell_session__return_loaded_value(request_id, own_loaded_value);
    if (!own_response) {
        ar_data__destroy(own_loaded_value);
        return NULL;
    }

    return own_response;  // Ownership transferred to caller
}

bool ar_shell_session__is_active(const ar_shell_session_t *ref_session) {
    if (!ref_session) {
        return false;
    }

    return ref_session->is_active;
}

ar_data_t* ar_shell_session__get_memory(const ar_shell_session_t *ref_session) {
    if (!ref_session) {
        return NULL;
    }

    return ref_session->own_memory;
}
