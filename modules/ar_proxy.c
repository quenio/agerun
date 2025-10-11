#include "ar_proxy.h"
#include "ar_heap.h"
#include <stdlib.h>

struct ar_proxy_s {
    ar_log_t *ref_log;
    const char *type;
};

ar_proxy_t* ar_proxy__create(ar_log_t *ref_log, const char *type) {
    ar_proxy_t *own_proxy = AR__HEAP__MALLOC(sizeof(ar_proxy_t), "proxy");
    if (!own_proxy) {
        return NULL;
    }

    own_proxy->ref_log = ref_log;
    own_proxy->type = type;

    return own_proxy;
}

void ar_proxy__destroy(ar_proxy_t *own_proxy) {
    if (!own_proxy) {
        return;
    }

    AR__HEAP__FREE(own_proxy);
}

ar_log_t* ar_proxy__get_log(const ar_proxy_t *ref_proxy) {
    if (!ref_proxy) {
        return NULL;
    }

    return ref_proxy->ref_log;
}

const char* ar_proxy__get_type(const ar_proxy_t *ref_proxy) {
    if (!ref_proxy) {
        return NULL;
    }

    return ref_proxy->type;
}
