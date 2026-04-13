#include "ar_shell.h"
#include "ar_agency.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_methodology.h"
#include "ar_shell_delegate.h"

#include <stdio.h>
#include <string.h>

struct ar_shell_s {
    ar_shell_mode_t default_mode;
    ar_system_t *own_system;
    ar_list_t *own_sessions;
};

ar_shell_t* ar_shell__create(ar_shell_mode_t default_mode) {
    ar_shell_t *own_shell = AR__HEAP__MALLOC(sizeof(ar_shell_t), "shell");
    if (!own_shell) {
        return NULL;
    }

    own_shell->default_mode = default_mode;
    own_shell->own_system = ar_system__create();
    own_shell->own_sessions = ar_list__create();
    if (!own_shell->own_system || !own_shell->own_sessions) {
        ar_shell__destroy(own_shell);
        return NULL;
    }

    ar_system__init(own_shell->own_system, NULL, NULL);
    return own_shell;
}

void ar_shell__destroy(ar_shell_t *own_shell) {
    ar_shell_session_t *own_session;

    if (!own_shell) {
        return;
    }

    if (own_shell->own_sessions) {
        while ((own_session = ar_list__remove_first(own_shell->own_sessions)) != NULL) {
            ar_shell_session__destroy(own_session);
        }
        ar_list__destroy(own_shell->own_sessions);
    }

    if (own_shell->own_system) {
        ar_system__shutdown(own_shell->own_system);
        ar_system__destroy(own_shell->own_system);
    }

    AR__HEAP__FREE(own_shell);
}

ar_shell_mode_t ar_shell__get_default_mode(const ar_shell_t *ref_shell) {
    if (!ref_shell) {
        return AR_SHELL_MODE__NORMAL;
    }

    return ref_shell->default_mode;
}

ar_system_t* ar_shell__get_system(const ar_shell_t *ref_shell) {
    if (!ref_shell) {
        return NULL;
    }

    return ref_shell->own_system;
}

size_t ar_shell__get_session_count(const ar_shell_t *ref_shell) {
    if (!ref_shell || !ref_shell->own_sessions) {
        return 0;
    }

    return ar_list__count(ref_shell->own_sessions);
}

ar_shell_session_t* ar_shell__start_session(ar_shell_t *mut_shell, ar_shell_mode_t mode) {
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_shell_session_t *own_session;
    int64_t own_agent_id;
    int64_t own_session_id;

    if (!mut_shell || !mut_shell->own_system || !mut_shell->own_sessions) {
        return NULL;
    }

    mut_agency = ar_system__get_agency(mut_shell->own_system);
    if (!mut_agency) {
        return NULL;
    }

    mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology || !ar_methodology__register_shell_method(mut_methodology)) {
        return NULL;
    }

    own_session_id = (int64_t)ar_list__count(mut_shell->own_sessions) + 1;
    own_session = ar_shell_session__create(own_session_id, mode);
    if (!own_session) {
        return NULL;
    }

    own_agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    if (own_agent_id <= 0 || !ar_shell_session__activate(own_session, own_agent_id)) {
        if (own_agent_id > 0) {
            ar_agency__destroy_agent(mut_agency, own_agent_id);
        }
        ar_shell_session__destroy(own_session);
        return NULL;
    }

    if (!ar_list__add_last(mut_shell->own_sessions, own_session)) {
        ar_agency__destroy_agent(mut_agency, own_agent_id);
        ar_shell_session__destroy(own_session);
        return NULL;
    }

    return own_session;
}

int ar_shell__main(int argc, char **argv) {
    ar_shell_mode_t default_mode = AR_SHELL_MODE__NORMAL;
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_shell_delegate_t *own_delegate;

    if (argc > 1 && argv && argv[1] && strcmp(argv[1], "--verbose") == 0) {
        default_mode = AR_SHELL_MODE__VERBOSE;
    }

    own_shell = ar_shell__create(default_mode);
    if (!own_shell) {
        return 1;
    }

    ref_session = ar_shell__start_session(own_shell, default_mode);
    if (!ref_session) {
        ar_shell__destroy(own_shell);
        return 1;
    }

    own_delegate = ar_shell_delegate__create(
        ar_system__get_log(own_shell->own_system),
        ref_session,
        ar_shell_session__get_agent_id(ref_session));
    if (!own_delegate) {
        ar_shell__destroy(own_shell);
        return 1;
    }

    (void) ar_shell_delegate__process_input_stream(own_delegate, own_shell->own_system, stdin, stdout);

    ar_shell_delegate__destroy(own_delegate);
    ar_shell__destroy(own_shell);
    return 0;
}

#ifdef AR_SHELL__BUILD_MAIN
int main(int argc, char **argv) {
    return ar_shell__main(argc, argv);
}
#endif
