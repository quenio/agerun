#include "ar_delegate.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_data.h"
#include <stdbool.h>
#include <stdlib.h>

struct ar_delegate_s {
    ar_log_t *ref_log;
    const char *type;
    ar_list_t *own_message_queue;
};

ar_delegate_t* ar_delegate__create(ar_log_t *ref_log, const char *type) {
    ar_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_delegate_t), "delegate");
    if (!own_delegate) {
        return NULL;
    }

    own_delegate->ref_log = ref_log;
    own_delegate->type = type;
    own_delegate->own_message_queue = ar_list__create();

    return own_delegate;
}

void ar_delegate__destroy(ar_delegate_t *own_delegate) {
    if (!own_delegate) {
        return;
    }

    if (own_delegate->own_message_queue) {
        ar_data_t *own_msg = NULL;
        while ((own_msg = ar_list__remove_first(own_delegate->own_message_queue)) != NULL) {
            ar_data__destroy_if_owned(own_msg, own_delegate);
        }
        ar_list__destroy(own_delegate->own_message_queue);
    }

    AR__HEAP__FREE(own_delegate);
}

ar_log_t* ar_delegate__get_log(const ar_delegate_t *ref_delegate) {
    if (!ref_delegate) {
        return NULL;
    }

    return ref_delegate->ref_log;
}

const char* ar_delegate__get_type(const ar_delegate_t *ref_delegate) {
    if (!ref_delegate) {
        return NULL;
    }

    return ref_delegate->type;
}

bool ar_delegate__handle_message(ar_delegate_t *ref_delegate, ar_data_t *ref_message, int64_t sender_id) {
    (void)ref_message;
    (void)sender_id;

    if (!ref_delegate) {
        return false;
    }

    // No handler configured yet, return false
    return false;
}

bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (!mut_delegate || !own_message) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    if (!mut_delegate->own_message_queue) {
        ar_data__destroy_if_owned(own_message, mut_delegate);
        return false;
    }
    // Take ownership for the delegate before adding to queue
    ar_data__take_ownership(own_message, mut_delegate);
    bool result = ar_list__add_last(mut_delegate->own_message_queue, own_message);
    if (!result) {
        ar_data__destroy_if_owned(own_message, mut_delegate);
    }
    return result;
}

bool ar_delegate__has_messages(const ar_delegate_t *ref_delegate) {
    if (!ref_delegate || !ref_delegate->own_message_queue) {
        return false;
    }
    return ar_list__count(ref_delegate->own_message_queue) > 0;
}

ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    if (!mut_delegate || !mut_delegate->own_message_queue) {
        return NULL;
    }
    ar_data_t *own_msg = (ar_data_t*)ar_list__remove_first(mut_delegate->own_message_queue);
    if (own_msg) {
        // Drop ownership - caller must destroy
        ar_data__drop_ownership(own_msg, mut_delegate);
    }
    return own_msg;
}
