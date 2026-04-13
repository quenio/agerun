#include "ar_shell_session.h"
#include "ar_agency.h"
#include "ar_heap.h"
#include <string.h>

static const char *AR_SHELL_SESSION__ACTION__STORE_VALUE =
    "ar_shell_session__store_value";
static const char *AR_SHELL_SESSION__ACTION__LOAD_VALUE =
    "ar_shell_session__load_value";
static const char *AR_SHELL_SESSION__ACTION__RETURN_LOADED_VALUE =
    "ar_shell_session__return_loaded_value";
static const char *AR_SHELL_SESSION__ACTION__REPORT_OPERATION_FAILURE =
    "ar_shell_session__report_operation_failure";

typedef struct ar_shell_session_delegate_context_s {
    ar_shell_session_t *mut_session;
    ar_system_t *mut_system;
} ar_shell_session_delegate_context_t;

static const char* _memory_relative_path(const char *ref_path);
static bool _send_protocol_reply(
    const ar_shell_session_delegate_context_t *ref_context,
    int64_t sender_id,
    ar_data_t *own_reply);
static bool _handle_runtime_delegate_message(void *mut_context, ar_data_t *ref_message, int64_t sender_id);
static void _destroy_runtime_delegate_context(void *own_context);

struct ar_shell_session_s {
    int64_t session_id;
    ar_shell_mode_t mode;
    int64_t agent_id;
    bool is_active;
    ar_data_t *own_memory;
    ar_data_t *own_context;
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
    own_session->own_context = ar_data__create_map();
    if (!own_session->own_memory || !own_session->own_context) {
        if (own_session->own_memory) {
            ar_data__destroy(own_session->own_memory);
        }
        if (own_session->own_context) {
            ar_data__destroy(own_session->own_context);
        }
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
    if (own_session->own_context) {
        ar_data__destroy(own_session->own_context);
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

int64_t ar_shell_session__get_runtime_delegate_id(const ar_shell_session_t *ref_session) {
    if (!ref_session || ref_session->session_id <= 0) {
        return 0;
    }

    return -1000 - ref_session->session_id;
}

static bool _send_protocol_reply(
    const ar_shell_session_delegate_context_t *ref_context,
    int64_t sender_id,
    ar_data_t *own_reply) {
    ar_agency_t *mut_agency;

    if (!own_reply) {
        return false;
    }

    if (!ref_context || !ref_context->mut_system || sender_id <= 0) {
        ar_data__destroy(own_reply);
        return false;
    }

    mut_agency = ar_system__get_agency(ref_context->mut_system);
    if (!mut_agency) {
        ar_data__destroy(own_reply);
        return false;
    }

    if (!ar_agency__send_to_agent(mut_agency, sender_id, own_reply)) {
        ar_data__destroy(own_reply);
        return false;
    }

    return true;
}

static bool _handle_runtime_delegate_message(void *mut_context, ar_data_t *ref_message, int64_t sender_id) {
    ar_shell_session_delegate_context_t *mut_context_data;
    const char *ref_action;
    const char *ref_path;
    ar_data_t *ref_value;
    ar_data_t *own_value_copy;
    ar_data_t *own_reply;
    int request_id;

    mut_context_data = mut_context;
    if (!mut_context_data || !mut_context_data->mut_session || !ref_message ||
        ar_data__get_type(ref_message) != AR_DATA_TYPE__MAP) {
        return false;
    }

    ref_action = ar_data__get_map_string(ref_message, "action");
    if (!ref_action) {
        return false;
    }

    if (strcmp(ref_action, AR_SHELL_SESSION__ACTION__STORE_VALUE) == 0) {
        ref_path = ar_data__get_map_string(ref_message, "path");
        ref_value = ar_data__get_map_data(ref_message, "value");
        own_value_copy = ref_value ? ar_data__shallow_copy(ref_value) : NULL;
        if (!ref_path || !own_value_copy ||
            !ar_shell_session__store_value(mut_context_data->mut_session, ref_path, own_value_copy)) {
            if (own_value_copy) {
                ar_data__destroy(own_value_copy);
            }
            own_reply = ar_shell_session__report_operation_failure(0, "Shell session store failed");
            return _send_protocol_reply(mut_context_data, sender_id, own_reply);
        }
        return true;
    }

    if (strcmp(ref_action, AR_SHELL_SESSION__ACTION__LOAD_VALUE) == 0) {
        ref_path = ar_data__get_map_string(ref_message, "path");
        request_id = ar_data__get_map_integer(ref_message, "request_id");
        own_reply = ar_shell_session__load_value(mut_context_data->mut_session, ref_path, request_id);
        return _send_protocol_reply(mut_context_data, sender_id, own_reply);
    }

    own_reply = ar_shell_session__report_operation_failure(0, "Unknown shell session action");
    return _send_protocol_reply(mut_context_data, sender_id, own_reply);
}

static void _destroy_runtime_delegate_context(void *own_context) {
    AR__HEAP__FREE(own_context);
}

ar_delegate_t* ar_shell_session__create_runtime_delegate(
    ar_shell_session_t *mut_session,
    ar_system_t *mut_system) {
    ar_shell_session_delegate_context_t *own_context;

    if (!mut_session || !mut_system) {
        return NULL;
    }

    own_context = AR__HEAP__MALLOC(sizeof(ar_shell_session_delegate_context_t), "shell session delegate context");
    if (!own_context) {
        return NULL;
    }

    own_context->mut_session = mut_session;
    own_context->mut_system = mut_system;

    return ar_delegate__create_with_handler(
        ar_system__get_log(mut_system),
        "shell-session",
        _handle_runtime_delegate_message,
        own_context,
        _destroy_runtime_delegate_context);
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

ar_data_t* ar_shell_session__get_context(const ar_shell_session_t *ref_session) {
    if (!ref_session) {
        return NULL;
    }

    return ref_session->own_context;
}
