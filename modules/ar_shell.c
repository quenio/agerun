#include "ar_shell.h"
#include "ar_agency.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_methodology.h"
#include "ar_shell_delegate.h"
#include "ar_io.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

struct ar_shell_s {
    ar_shell_mode_t default_mode;
    ar_system_t *own_system;
    ar_list_t *own_sessions;
};

static ar_shell_mode_t _parse_startup_mode(int argc, char **argv);
static int _load_methods_from_directory(ar_methodology_t *mut_methodology);
static bool _register_shell_session_delegate(
    ar_shell_t *mut_shell,
    ar_shell_session_t *mut_session);
static int64_t _create_shell_receiving_agent(
    ar_shell_t *mut_shell,
    ar_shell_session_t *ref_session);
static bool _store_runtime_delegate_id_on_agent(
    ar_system_t *ref_system,
    int64_t agent_id,
    int64_t delegate_id);

static ar_shell_mode_t _parse_startup_mode(int argc, char **argv) {
    if (argc > 1 && argv && argv[1] && strcmp(argv[1], "--verbose") == 0) {
        return AR_SHELL_MODE__VERBOSE;
    }

    return AR_SHELL_MODE__NORMAL;
}

static int _load_methods_from_directory(ar_methodology_t *mut_methodology) {
    const char *ref_methods_dir;
    DIR *own_directory;
    struct dirent *ref_entry;
    int ref_loaded_count = 0;

    if (!mut_methodology) {
        return 0;
    }

    ref_methods_dir = getenv("AGERUN_METHODS_DIR");
    if (!ref_methods_dir) {
        ref_methods_dir = "../../methods";
    }

    own_directory = opendir(ref_methods_dir);
    if (!own_directory) {
        return 0;
    }

    while ((ref_entry = readdir(own_directory)) != NULL) {
        const char *ref_extension;
        char mut_method_name[256];
        char mut_version[32];
        char mut_filepath[2048];
        char *mut_last_hyphen;
        FILE *mut_file;
        ar_file_result_t ref_file_result;
        long ref_file_size;
        char *own_content;
        size_t ref_bytes_read;
        size_t ref_name_length;

        ref_extension = strrchr(ref_entry->d_name, '.');
        if (!ref_extension || strcmp(ref_extension, ".method") != 0) {
            continue;
        }

        ref_name_length = (size_t)(ref_extension - ref_entry->d_name);
        if (ref_name_length >= sizeof(mut_method_name)) {
            continue;
        }

        strncpy(mut_method_name, ref_entry->d_name, ref_name_length);
        mut_method_name[ref_name_length] = '\0';
        mut_last_hyphen = strrchr(mut_method_name, '-');
        if (!mut_last_hyphen) {
            continue;
        }

        *mut_last_hyphen = '\0';
        strncpy(mut_version, mut_last_hyphen + 1, sizeof(mut_version) - 1);
        mut_version[sizeof(mut_version) - 1] = '\0';
        snprintf(mut_filepath, sizeof(mut_filepath), "%s/%s", ref_methods_dir, ref_entry->d_name);

        mut_file = NULL;
        ref_file_result = ar_io__open_file(mut_filepath, "r", &mut_file);
        if (ref_file_result != AR_FILE_RESULT__SUCCESS) {
            continue;
        }

        fseek(mut_file, 0, SEEK_END);
        ref_file_size = ftell(mut_file);
        fseek(mut_file, 0, SEEK_SET);

        own_content = AR__HEAP__MALLOC((size_t)(ref_file_size + 1), "shell method file content");
        if (!own_content) {
            ar_io__close_file(mut_file, mut_filepath);
            continue;
        }

        ref_bytes_read = fread(own_content, 1, (size_t)ref_file_size, mut_file);
        own_content[ref_bytes_read] = '\0';
        ar_io__close_file(mut_file, mut_filepath);

        if (ar_methodology__create_method(mut_methodology, mut_method_name, own_content, mut_version)) {
            ref_loaded_count++;
        }

        AR__HEAP__FREE(own_content);
    }

    closedir(own_directory);
    return ref_loaded_count;
}

static bool _register_shell_session_delegate(
    ar_shell_t *mut_shell,
    ar_shell_session_t *mut_session) {
    ar_delegate_t *own_runtime_delegate;
    int64_t ref_delegate_id;

    if (!mut_shell || !mut_shell->own_system || !mut_session) {
        return false;
    }

    own_runtime_delegate = ar_shell_session__create_runtime_delegate(mut_session, mut_shell->own_system);
    ref_delegate_id = ar_shell_session__get_runtime_delegate_id(mut_session);
    if (!own_runtime_delegate || ref_delegate_id >= 0 ||
        !ar_system__register_delegate(mut_shell->own_system, ref_delegate_id, own_runtime_delegate)) {
        if (own_runtime_delegate) {
            ar_delegate__destroy(own_runtime_delegate);
        }
        return false;
    }

    return true;
}

static int64_t _create_shell_receiving_agent(
    ar_shell_t *mut_shell,
    ar_shell_session_t *ref_session) {
    ar_agency_t *mut_agency;

    if (!mut_shell || !mut_shell->own_system || !ref_session) {
        return 0;
    }

    mut_agency = ar_system__get_agency(mut_shell->own_system);
    if (!mut_agency) {
        return 0;
    }

    return ar_agency__create_agent(
        mut_agency,
        AR_SHELL_METHOD_NAME,
        AR_SHELL_METHOD_VERSION,
        ar_shell_session__get_context(ref_session));
}

static bool _store_runtime_delegate_id_on_agent(
    ar_system_t *ref_system,
    int64_t agent_id,
    int64_t delegate_id) {
    ar_agency_t *mut_agency;
    ar_data_t *mut_agent_memory;

    if (!ref_system || agent_id <= 0) {
        return false;
    }

    mut_agency = ar_system__get_agency(ref_system);
    if (!mut_agency) {
        return false;
    }

    mut_agent_memory = ar_agency__get_agent_mutable_memory(mut_agency, agent_id);
    if (!mut_agent_memory) {
        return false;
    }

    return ar_data__set_map_integer(mut_agent_memory, "shell_session_delegate_id", (int)delegate_id);
}

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
    (void) _load_methods_from_directory(ar_agency__get_methodology(ar_system__get_agency(own_shell->own_system)));
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
    int64_t ref_delegate_id;

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

    if (!_register_shell_session_delegate(mut_shell, own_session)) {
        ar_shell_session__destroy(own_session);
        return NULL;
    }

    own_agent_id = _create_shell_receiving_agent(mut_shell, own_session);
    if (own_agent_id <= 0 || !ar_shell_session__activate(own_session, own_agent_id)) {
        if (own_agent_id > 0) {
            ar_agency__destroy_agent(mut_agency, own_agent_id);
        }
        ar_shell_session__destroy(own_session);
        return NULL;
    }

    ref_delegate_id = ar_shell_session__get_runtime_delegate_id(own_session);
    if (!_store_runtime_delegate_id_on_agent(mut_shell->own_system, own_agent_id, ref_delegate_id)) {
        ar_agency__destroy_agent(mut_agency, own_agent_id);
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
    ar_shell_mode_t default_mode;
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_shell_delegate_t *own_delegate;

    default_mode = _parse_startup_mode(argc, argv);

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
