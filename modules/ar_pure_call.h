#ifndef AGERUN_PURE_CALL_H
#define AGERUN_PURE_CALL_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file ar_pure_call.h
 * @brief Shared metadata for registered pure expression calls.
 *
 * The pure-call registry is the single source of truth for expression-call names
 * and expected arities. A registered entry is classified as a pure call.
 */

/* Opaque pure-call metadata entry */
typedef struct ar_pure_call_s ar_pure_call_t;

/**
 * Registered pure-call identity.
 *
 * The identity is part of the abstract language model, not a storage detail.
 */
typedef enum {
    AR_PURE_CALL_TYPE__UNKNOWN = 0,
    AR_PURE_CALL_TYPE__PARSE,
    AR_PURE_CALL_TYPE__BUILD,
    AR_PURE_CALL_TYPE__IF,
    AR_PURE_CALL_TYPE__HEAD,
    AR_PURE_CALL_TYPE__TAIL,
    AR_PURE_CALL_TYPE__APPEND
} ar_pure_call_type_t;

/**
 * Find a registered pure call by name.
 *
 * @param ref_name Pure-call name to look up (borrowed reference)
 * @return Borrowed metadata entry, or NULL when the name is not registered
 */
const ar_pure_call_t* ar_pure_call__find(const char *ref_name);

/**
 * Check whether a name is a registered pure call.
 *
 * @param ref_name Name to classify (borrowed reference)
 * @return true when the name is registered as a pure call, otherwise false
 */
bool ar_pure_call__is_registered(const char *ref_name);

/**
 * Get the name for a pure-call metadata entry.
 *
 * @param ref_call Pure-call metadata entry (borrowed reference)
 * @return Borrowed call name, or NULL when ref_call is NULL
 */
const char* ar_pure_call__get_name(const ar_pure_call_t *ref_call);

/**
 * Get the expected argument count for a pure-call metadata entry.
 *
 * @param ref_call Pure-call metadata entry (borrowed reference)
 * @return Expected argument count, or 0 when ref_call is NULL
 */
size_t ar_pure_call__get_arity(const ar_pure_call_t *ref_call);

/**
 * Get the registered call identity for a pure-call metadata entry.
 *
 * @param ref_call Pure-call metadata entry (borrowed reference)
 * @return Registered call identity, or AR_PURE_CALL_TYPE__UNKNOWN when ref_call is NULL
 */
ar_pure_call_type_t ar_pure_call__get_type(const ar_pure_call_t *ref_call);

/**
 * Count registered pure calls.
 *
 * @return Number of registered pure calls
 */
size_t ar_pure_call__count(void);

/**
 * Get a registered pure call by registry index.
 *
 * @param index Zero-based registry index
 * @return Borrowed metadata entry, or NULL when index is out of range
 */
const ar_pure_call_t* ar_pure_call__get_at(size_t index);

#endif /* AGERUN_PURE_CALL_H */
