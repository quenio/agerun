#include "ar_shell_delegate.h"
#include "ar_agency.h"
#include "ar_heap.h"
#include "ar_method.h"
#include <inttypes.h>
#include <string.h>

static void _trim_line_endings(char *mut_text);
static bool _report_handoff_acknowledgement(
    const ar_shell_delegate_t *ref_delegate,
    FILE *mut_output,
    bool did_handoff_succeed);
static bool _queue_input_envelope(
    ar_agency_t *mut_agency,
    int64_t agent_id,
    const char *ref_text);
static bool _is_list_agents_command(const char *ref_text);
static bool _render_active_agents_listing(FILE *mut_output, ar_system_t *mut_system);
static bool _has_runtime_delegate_binding(ar_shell_delegate_t *mut_delegate, ar_system_t *mut_system);
static void _process_runtime_interaction(ar_shell_delegate_t *mut_delegate, ar_system_t *mut_system);
static void _close_session_after_eof(ar_shell_delegate_t *mut_delegate, ar_system_t *mut_system);
static bool _forward_trimmed_line(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    FILE *mut_output,
    const char *ref_text);

struct ar_shell_delegate_s {
    ar_log_t *ref_log;
    ar_shell_session_t *ref_session;
    int64_t agent_id;
};

ar_shell_delegate_t* ar_shell_delegate__create(
    ar_log_t *ref_log,
    ar_shell_session_t *ref_session,
    int64_t agent_id) {
    ar_shell_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_shell_delegate_t), "shell delegate");
    if (!own_delegate) {
        return NULL;
    }

    own_delegate->ref_log = ref_log;
    own_delegate->ref_session = ref_session;
    own_delegate->agent_id = agent_id;
    return own_delegate;
}

void ar_shell_delegate__destroy(ar_shell_delegate_t *own_delegate) {
    if (!own_delegate) {
        return;
    }

    AR__HEAP__FREE(own_delegate);
}

int64_t ar_shell_delegate__get_agent_id(const ar_shell_delegate_t *ref_delegate) {
    if (!ref_delegate) {
        return 0;
    }

    return ref_delegate->agent_id;
}

ar_data_t* ar_shell_delegate__create_input_envelope(const char *ref_text) {
    ar_data_t *own_envelope;

    if (!ref_text) {
        return NULL;
    }

    own_envelope = ar_data__create_map();
    if (!own_envelope) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_envelope, "text", ref_text)) {
        ar_data__destroy(own_envelope);
        return NULL;
    }

    return own_envelope;  // Ownership transferred to caller
}

static void _trim_line_endings(char *mut_text) {
    size_t ref_length;

    if (!mut_text) {
        return;
    }

    ref_length = strlen(mut_text);
    while (ref_length > 0 &&
           (mut_text[ref_length - 1] == '\n' || mut_text[ref_length - 1] == '\r')) {
        mut_text[ref_length - 1] = '\0';
        ref_length--;
    }
}

static bool _report_handoff_acknowledgement(
    const ar_shell_delegate_t *ref_delegate,
    FILE *mut_output,
    bool did_handoff_succeed) {
    int ref_result;

    if (!ref_delegate || !mut_output || !ref_delegate->ref_session) {
        return false;
    }

    if (ar_shell_session__get_mode(ref_delegate->ref_session) == AR_SHELL_MODE__VERBOSE) {
        ref_result = fprintf(
            mut_output,
            did_handoff_succeed ? "handoff ok agent_id=%" PRId64 "\n"
                                : "handoff failed agent_id=%" PRId64 "\n",
            ref_delegate->agent_id);
    } else {
        ref_result = fprintf(mut_output, did_handoff_succeed ? "handoff ok\n" : "handoff failed\n");
    }

    if (ref_result < 0) {
        return false;
    }

    return fflush(mut_output) == 0;
}

static bool _queue_input_envelope(
    ar_agency_t *mut_agency,
    int64_t agent_id,
    const char *ref_text) {
    ar_data_t *own_envelope;

    if (!mut_agency || agent_id <= 0 || !ref_text) {
        return false;
    }

    own_envelope = ar_shell_delegate__create_input_envelope(ref_text);
    if (!own_envelope) {
        return false;
    }

    return ar_agency__send_to_agent(mut_agency, agent_id, own_envelope);
}

bool ar_shell_delegate__forward_input(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    const char *ref_text) {
    ar_agency_t *mut_agency;

    if (!mut_delegate || !mut_system || !ref_text) {
        return false;
    }

    mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        return false;
    }

    return _queue_input_envelope(mut_agency, mut_delegate->agent_id, ref_text);
}

static bool _is_list_agents_command(const char *ref_text) {
    if (!ref_text) {
        return false;
    }

    return strcmp(ref_text, "agents") == 0 || strcmp(ref_text, "list agents") == 0;
}

static bool _render_active_agents_listing(FILE *mut_output, ar_system_t *mut_system) {
    ar_agency_t *mut_agency;
    int64_t agent_id;
    bool has_rendered_any;

    if (!mut_output || !mut_system) {
        return false;
    }

    mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        return false;
    }

    has_rendered_any = false;
    agent_id = ar_agency__get_first_agent(mut_agency);
    while (agent_id != 0) {
        const ar_method_t *ref_method;
        const char *ref_method_name;
        const char *ref_method_version;
        int64_t next_agent_id;

        next_agent_id = ar_agency__get_next_agent(mut_agency, agent_id);
        ref_method = ar_agency__get_agent_method(mut_agency, agent_id);
        ref_method_name = ref_method ? ar_method__get_name(ref_method) : NULL;
        ref_method_version = ref_method ? ar_method__get_version(ref_method) : NULL;

        if (fprintf(mut_output,
                    "agent id=%" PRId64 " method=%s version=%s\n",
                    agent_id,
                    ref_method_name ? ref_method_name : "<unknown>",
                    ref_method_version ? ref_method_version : "<unknown>") < 0) {
            return false;
        }

        has_rendered_any = true;
        agent_id = next_agent_id;
    }

    if (!has_rendered_any) {
        if (fprintf(mut_output, "agent list empty\n") < 0) {
            return false;
        }
    }

    return fflush(mut_output) == 0;
}

static bool _has_runtime_delegate_binding(ar_shell_delegate_t *mut_delegate, ar_system_t *mut_system) {
    ar_agency_t *mut_agency;
    const ar_data_t *ref_agent_memory;

    if (!mut_delegate || !mut_system || mut_delegate->agent_id <= 0) {
        return false;
    }

    mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        return false;
    }

    ref_agent_memory = ar_agency__get_agent_memory(mut_agency, mut_delegate->agent_id);
    if (!ref_agent_memory) {
        return false;
    }

    return ar_data__get_map_integer(ref_agent_memory, "shell_session_delegate_id") < 0;
}

static void _process_runtime_interaction(ar_shell_delegate_t *mut_delegate, ar_system_t *mut_system) {
    if (!mut_delegate || !mut_system || !_has_runtime_delegate_binding(mut_delegate, mut_system)) {
        return;
    }

    (void) ar_system__process_all_messages(mut_system);
}

static void _close_session_after_eof(ar_shell_delegate_t *mut_delegate, ar_system_t *mut_system) {
    ar_agency_t *mut_agency;

    if (!mut_delegate || !mut_delegate->ref_session || !mut_system ||
        !_has_runtime_delegate_binding(mut_delegate, mut_system)) {
        return;
    }

    ar_shell_session__close(mut_delegate->ref_session);

    mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency || mut_delegate->agent_id <= 0) {
        return;
    }

    if (ar_agency__agent_exists(mut_agency, mut_delegate->agent_id)) {
        (void) ar_agency__destroy_agent(mut_agency, mut_delegate->agent_id);
    }
}

static bool _forward_trimmed_line(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    FILE *mut_output,
    const char *ref_text) {
    bool did_handoff_succeed;

    if (!mut_delegate || !mut_system || !mut_output || !ref_text) {
        return false;
    }

    if (_is_list_agents_command(ref_text)) {
        did_handoff_succeed = true;
        if (!_report_handoff_acknowledgement(mut_delegate, mut_output, did_handoff_succeed)) {
            return false;
        }
        return _render_active_agents_listing(mut_output, mut_system);
    }

    did_handoff_succeed = ar_shell_delegate__forward_input(mut_delegate, mut_system, ref_text);
    _report_handoff_acknowledgement(mut_delegate, mut_output, did_handoff_succeed);
    if (did_handoff_succeed) {
        _process_runtime_interaction(mut_delegate, mut_system);
    }
    return true;
}

size_t ar_shell_delegate__process_input_stream(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    FILE *mut_input,
    FILE *mut_output) {
    char mut_line_buffer[4096];
    size_t ref_processed_count = 0;

    if (!mut_delegate || !mut_system || !mut_input || !mut_output) {
        return 0;
    }

    ar_shell_session__bind_output(mut_delegate->ref_session, mut_output);

    while (fgets(mut_line_buffer, sizeof(mut_line_buffer), mut_input) != NULL) {
        _trim_line_endings(mut_line_buffer);
        if (_forward_trimmed_line(mut_delegate, mut_system, mut_output, mut_line_buffer)) {
            ref_processed_count++;
        }
    }

    _close_session_after_eof(mut_delegate, mut_system);
    return ref_processed_count;
}
