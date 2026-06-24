#include "ar_pure_call.h"

#include <string.h>

struct ar_pure_call_s {
    ar_pure_call_type_t type;
    const char *name;
    size_t arity;
};

static const ar_pure_call_t PURE_CALLS[] = {
    {AR_PURE_CALL_TYPE__PARSE, "parse", 2},
    {AR_PURE_CALL_TYPE__BUILD, "build", 2},
    {AR_PURE_CALL_TYPE__IF, "if", 3},
    {AR_PURE_CALL_TYPE__HEAD, "head", 1},
    {AR_PURE_CALL_TYPE__TAIL, "tail", 1},
    {AR_PURE_CALL_TYPE__APPEND, "append", 2}
};

#define PURE_CALL_COUNT (sizeof(PURE_CALLS) / sizeof(PURE_CALLS[0]))

const ar_pure_call_t* ar_pure_call__find(const char *ref_name) {
    if (ref_name == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < PURE_CALL_COUNT; i++) {
        if (strcmp(ref_name, PURE_CALLS[i].name) == 0) {
            return &PURE_CALLS[i];
        }
    }

    return NULL;
}

bool ar_pure_call__is_registered(const char *ref_name) {
    return ar_pure_call__find(ref_name) != NULL;
}

const char* ar_pure_call__get_name(const ar_pure_call_t *ref_call) {
    if (ref_call == NULL) {
        return NULL;
    }

    return ref_call->name;
}

size_t ar_pure_call__get_arity(const ar_pure_call_t *ref_call) {
    if (ref_call == NULL) {
        return 0;
    }

    return ref_call->arity;
}

ar_pure_call_type_t ar_pure_call__get_type(const ar_pure_call_t *ref_call) {
    if (ref_call == NULL) {
        return AR_PURE_CALL_TYPE__UNKNOWN;
    }

    return ref_call->type;
}

size_t ar_pure_call__count(void) {
    return PURE_CALL_COUNT;
}

const ar_pure_call_t* ar_pure_call__get_at(size_t index) {
    if (index >= PURE_CALL_COUNT) {
        return NULL;
    }

    return &PURE_CALLS[index];
}
