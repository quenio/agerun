/* AgeRun Delegation Implementation */
#include "ar_delegation.h"
#include "ar_delegate_registry.h"
#include "ar_data.h"
#include "ar_heap.h"
#include <stdbool.h>

/* Delegation structure */
struct ar_delegation_s {
    bool is_initialized;
    ar_log_t *ref_log;                        /* Borrowed reference from system */
    ar_delegate_registry_t *own_registry;     /* Owned by the delegation */
};

ar_delegation_t* ar_delegation__create(ar_log_t *ref_log) {
    ar_delegation_t *own_delegation = AR__HEAP__MALLOC(sizeof(ar_delegation_t), "delegation");
    if (!own_delegation) {
        return NULL;
    }

    own_delegation->is_initialized = true;
    own_delegation->ref_log = ref_log;

    // Create owned registry
    own_delegation->own_registry = ar_delegate_registry__create();
    if (!own_delegation->own_registry) {
        if (ref_log) {
            ar_log__error(ref_log, "Delegation: Failed to create delegate registry");
        }
        AR__HEAP__FREE(own_delegation);
        return NULL;
    }

    return own_delegation;
}

void ar_delegation__destroy(ar_delegation_t *own_delegation) {
    if (!own_delegation) {
        return;
    }

    // Destroy owned registry before freeing struct
    if (own_delegation->own_registry) {
        ar_delegate_registry__destroy(own_delegation->own_registry);
    }

    AR__HEAP__FREE(own_delegation);
}

ar_delegate_registry_t* ar_delegation__get_registry(const ar_delegation_t *ref_delegation) {
    if (!ref_delegation || !ref_delegation->is_initialized) {
        return NULL;
    }
    return ref_delegation->own_registry;
}

bool ar_delegation__register_delegate(ar_delegation_t *mut_delegation,
                                       int64_t delegate_id,
                                       ar_delegate_t *own_delegate) {
    if (!mut_delegation || !mut_delegation->is_initialized || !mut_delegation->own_registry) {
        return false;
    }

    // Delegate to the registry
    return ar_delegate_registry__register(mut_delegation->own_registry, delegate_id, own_delegate);
}

bool ar_delegation__send_to_delegate(ar_delegation_t *mut_delegation,
                                      int64_t delegate_id,
                                      ar_data_t *own_message) {
    if (!mut_delegation || !mut_delegation->own_registry) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    ar_delegate_t *mut_delegate = ar_delegate_registry__find(mut_delegation->own_registry, delegate_id);
    if (!mut_delegate) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    return ar_delegate__send(mut_delegate, own_message);
}

bool ar_delegation__delegate_has_messages(ar_delegation_t *ref_delegation,
                                           int64_t delegate_id) {
    if (!ref_delegation || !ref_delegation->own_registry) {
        return false;
    }
    ar_delegate_t *ref_delegate = ar_delegate_registry__find(ref_delegation->own_registry, delegate_id);
    if (!ref_delegate) {
        return false;
    }
    return ar_delegate__has_messages(ref_delegate);
}

ar_data_t* ar_delegation__take_delegate_message(ar_delegation_t *mut_delegation,
                                                 int64_t delegate_id) {
    if (!mut_delegation || !mut_delegation->own_registry) {
        return NULL;
    }
    ar_delegate_t *mut_delegate = ar_delegate_registry__find(mut_delegation->own_registry, delegate_id);
    if (!mut_delegate) {
        return NULL;
    }
    return ar_delegate__take_message(mut_delegate);
}
