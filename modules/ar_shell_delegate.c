#include "ar_shell_delegate.h"
#include "ar_agency.h"
#include "ar_heap.h"
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

static bool _forward_trimmed_line(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    FILE *mut_output,
    const char *ref_text) {
    bool did_handoff_succeed;

    if (!mut_delegate || !mut_system || !mut_output || !ref_text) {
        return false;
    }

    did_handoff_succeed = ar_shell_delegate__forward_input(mut_delegate, mut_system, ref_text);
    _report_handoff_acknowledgement(mut_delegate, mut_output, did_handoff_succeed);
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

    while (fgets(mut_line_buffer, sizeof(mut_line_buffer), mut_input) != NULL) {
        _trim_line_endings(mut_line_buffer);
        if (_forward_trimmed_line(mut_delegate, mut_system, mut_output, mut_line_buffer)) {
            ref_processed_count++;
        }
    }

    return ref_processed_count;
}
