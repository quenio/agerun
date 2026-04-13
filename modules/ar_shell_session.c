#include "ar_shell_session.h"
#include "ar_heap.h"

struct ar_shell_session_s {
    int64_t session_id;
    ar_shell_mode_t mode;
    int64_t agent_id;
    bool is_active;
    ar_data_t *own_memory;
};

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
