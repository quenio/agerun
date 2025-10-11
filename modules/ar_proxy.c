#include "ar_proxy.h"
#include "ar_heap.h"
#include <stdlib.h>

struct ar_proxy_s {
    /* Placeholder for future proxy state */
    int placeholder;
};

ar_proxy_t* ar_proxy__create(void) {
    ar_proxy_t *own_proxy = AR__HEAP__MALLOC(sizeof(ar_proxy_t), "proxy");
    if (!own_proxy) {
        return NULL;
    }

    own_proxy->placeholder = 0;

    return own_proxy;
}

void ar_proxy__destroy(ar_proxy_t *own_proxy) {
    if (!own_proxy) {
        return;
    }

    AR__HEAP__FREE(own_proxy);
}
