#include "ar_shell_delegate.h"
#include "ar_agency.h"
#include "ar_heap.h"

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

bool ar_shell_delegate__forward_input(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    const char *ref_text) {
    ar_agency_t *mut_agency;
    ar_data_t *own_envelope;

    if (!mut_delegate || !mut_system || !ref_text) {
        return false;
    }

    mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        return false;
    }

    own_envelope = ar_shell_delegate__create_input_envelope(ref_text);
    if (!own_envelope) {
        return false;
    }

    return ar_agency__send_to_agent(mut_agency, mut_delegate->agent_id, own_envelope);
}
