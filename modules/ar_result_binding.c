#include "ar_result_binding.h"

#include <string.h>

static bool _targets_protected_memory_self(const char *ref_result_path) {
    if (!ref_result_path) {
        return false;
    }

    return strcmp(ref_result_path, "memory.self") == 0 ||
           strncmp(ref_result_path, "memory.self.", 12) == 0;
}

bool ar_result_binding__validate_target(
    ar_log_t *ref_log,
    const char *ref_result_path
) {
    if (_targets_protected_memory_self(ref_result_path)) {
        ar_log__error(ref_log, "memory.self is agency-managed and cannot be assigned");
        return false;
    }

    return true;
}

bool ar_result_binding__bind(
    ar_log_t *ref_log,
    const ar_frame_t *ref_frame,
    const char *ref_result_path,
    ar_data_t *own_result
) {
    if (!own_result) {
        return false;
    }

    if (!ar_result_binding__validate_target(ref_log, ref_result_path)) {
        ar_data__destroy(own_result);
        return false;
    }

    bool did_bind = false;
    if (ref_frame && ref_result_path) {
        ar_data_t *mut_memory = ar_frame__get_memory(ref_frame);
        did_bind = ar_data__set_map_data_if_root_matched(
            mut_memory,
            "memory",
            ref_result_path,
            own_result
        );
    }

    if (!did_bind) {
        ar_data__destroy(own_result);
    }

    return did_bind;
}
