#include "ar_delegate.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_data.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AR_DELEGATE__NO_EXPLICIT_SENDER "__no_explicit_sender__"

static bool _enqueue_message(
    ar_delegate_t *mut_delegate,
    ar_data_t *own_message,
    bool has_explicit_sender,
    int64_t sender_id);

struct ar_delegate_s {
    ar_log_t *ref_log;
    const char *type;
    ar_list_t *own_message_queue;
    ar_list_t *own_sender_queue;
    ar_delegate_message_handler_fn ref_handler;
    void *own_handler_context;
    ar_delegate_context_destroy_fn ref_destroy_context;
};

static bool _enqueue_message(
    ar_delegate_t *mut_delegate,
    ar_data_t *own_message,
    bool has_explicit_sender,
    int64_t sender_id
) {
    ar_data_t *own_sender_data;
    char mut_sender_buffer[32];

    if (!mut_delegate || !own_message || !mut_delegate->own_message_queue || !mut_delegate->own_sender_queue) {
        return false;
    }

    if (has_explicit_sender) {
        snprintf(mut_sender_buffer, sizeof(mut_sender_buffer), "%" PRId64, sender_id);
        own_sender_data = ar_data__create_string(mut_sender_buffer);
    } else {
        own_sender_data = ar_data__create_string(AR_DELEGATE__NO_EXPLICIT_SENDER);
    }
    if (!own_sender_data) {
        return false;
    }

    if (!ar_list__add_last(mut_delegate->own_message_queue, own_message)) {
        ar_data__destroy(own_sender_data);
        return false;
    }

    if (!ar_list__add_last(mut_delegate->own_sender_queue, own_sender_data)) {
        ar_list__remove_last(mut_delegate->own_message_queue);
        ar_data__destroy(own_sender_data);
        return false;
    }

    return true;
}

ar_delegate_t* ar_delegate__create_with_handler(
    ar_log_t *ref_log,
    const char *type,
    ar_delegate_message_handler_fn ref_handler,
    void *own_handler_context,
    ar_delegate_context_destroy_fn ref_destroy_context
) {
    ar_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_delegate_t), "delegate");
    if (!own_delegate) {
        return NULL;
    }

    own_delegate->ref_log = ref_log;
    own_delegate->type = type;
    own_delegate->ref_handler = ref_handler;
    own_delegate->own_handler_context = own_handler_context;
    own_delegate->ref_destroy_context = ref_destroy_context;
    own_delegate->own_message_queue = ar_list__create();
    own_delegate->own_sender_queue = ar_list__create();
    if (!own_delegate->own_message_queue || !own_delegate->own_sender_queue) {
        if (own_delegate->own_message_queue) {
            ar_list__destroy(own_delegate->own_message_queue);
        }
        if (own_delegate->own_sender_queue) {
            ar_list__destroy(own_delegate->own_sender_queue);
        }
        AR__HEAP__FREE(own_delegate);
        return NULL;
    }

    return own_delegate;
}

ar_delegate_t* ar_delegate__create(ar_log_t *ref_log, const char *type) {
    return ar_delegate__create_with_handler(ref_log, type, NULL, NULL, NULL);
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

    if (own_delegate->own_sender_queue) {
        ar_data_t *own_sender = NULL;
        while ((own_sender = ar_list__remove_first(own_delegate->own_sender_queue)) != NULL) {
            ar_data__destroy(own_sender);
        }
        ar_list__destroy(own_delegate->own_sender_queue);
    }

    if (own_delegate->ref_destroy_context && own_delegate->own_handler_context) {
        own_delegate->ref_destroy_context(own_delegate->own_handler_context);
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
    if (!ref_delegate || !ref_delegate->ref_handler) {
        return false;
    }

    return ref_delegate->ref_handler(ref_delegate->own_handler_context, ref_message, sender_id);
}

bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (!mut_delegate || !own_message) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }

    ar_data__take_ownership(own_message, mut_delegate);
    if (!_enqueue_message(mut_delegate, own_message, false, 0)) {
        ar_data__destroy_if_owned(own_message, mut_delegate);
        return false;
    }

    return true;
}

bool ar_delegate__send_with_sender(
    ar_delegate_t *mut_delegate,
    ar_data_t *own_message,
    int64_t sender_id
) {
    if (!mut_delegate || !own_message) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }

    ar_data__take_ownership(own_message, mut_delegate);
    if (!_enqueue_message(mut_delegate, own_message, true, sender_id)) {
        ar_data__destroy_if_owned(own_message, mut_delegate);
        return false;
    }

    return true;
}

bool ar_delegate__send_from_owner(
    ar_delegate_t *mut_delegate,
    ar_data_t *mut_message,
    const void *ref_from_owner
) {
    if (!mut_delegate || !mut_message || !ref_from_owner ||
        !mut_delegate->own_message_queue || !mut_delegate->own_sender_queue) {
        return false;
    }

    if (!ar_data__transfer_ownership(mut_message, ref_from_owner, mut_delegate)) {
        return false;
    }

    if (!_enqueue_message(mut_delegate, mut_message, false, 0)) {
        ar_data__transfer_ownership(mut_message, mut_delegate, ref_from_owner);
        return false;
    }

    return true;
}

bool ar_delegate__send_from_owner_with_sender(
    ar_delegate_t *mut_delegate,
    ar_data_t *mut_message,
    const void *ref_from_owner,
    int64_t sender_id
) {
    if (!mut_delegate || !mut_message || !ref_from_owner ||
        !mut_delegate->own_message_queue || !mut_delegate->own_sender_queue) {
        return false;
    }

    if (!ar_data__transfer_ownership(mut_message, ref_from_owner, mut_delegate)) {
        return false;
    }

    if (!_enqueue_message(mut_delegate, mut_message, true, sender_id)) {
        ar_data__transfer_ownership(mut_message, mut_delegate, ref_from_owner);
        return false;
    }

    return true;
}

bool ar_delegate__has_messages(const ar_delegate_t *ref_delegate) {
    if (!ref_delegate || !ref_delegate->own_message_queue) {
        return false;
    }
    return ar_list__count(ref_delegate->own_message_queue) > 0;
}

ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    return ar_delegate__take_message_with_sender(mut_delegate, NULL, NULL);
}

ar_data_t* ar_delegate__take_message_with_sender(
    ar_delegate_t *mut_delegate,
    int64_t *mut_sender_id,
    bool *mut_has_explicit_sender
) {
    ar_data_t *own_msg;
    ar_data_t *own_sender_data;
    int64_t sender_value = 0;
    bool has_explicit_sender = false;

    if (!mut_delegate || !mut_delegate->own_message_queue || !mut_delegate->own_sender_queue) {
        return NULL;
    }

    own_msg = (ar_data_t*)ar_list__remove_first(mut_delegate->own_message_queue);
    if (!own_msg) {
        return NULL;
    }

    own_sender_data = (ar_data_t*)ar_list__remove_first(mut_delegate->own_sender_queue);
    if (own_sender_data) {
        const char *ref_sender_string = ar_data__get_string(own_sender_data);
        has_explicit_sender =
            (ref_sender_string != NULL && strcmp(ref_sender_string, AR_DELEGATE__NO_EXPLICIT_SENDER) != 0);
        if (has_explicit_sender) {
            sender_value = strtoll(ref_sender_string, NULL, 10);
        }
        ar_data__destroy(own_sender_data);
    }

    if (mut_sender_id) {
        *mut_sender_id = has_explicit_sender ? sender_value : 0;
    }
    if (mut_has_explicit_sender) {
        *mut_has_explicit_sender = has_explicit_sender;
    }

    ar_data__drop_ownership(own_msg, mut_delegate);
    return own_msg;
}
