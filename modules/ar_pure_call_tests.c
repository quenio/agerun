#include <stdio.h>
#include <string.h>

#include "ar_assert.h"
#include "ar_pure_call.h"

typedef struct {
    const char *name;
    size_t arity;
    ar_pure_call_type_t type;
} expected_pure_call_t;

static const expected_pure_call_t EXPECTED_PURE_CALLS[] = {
    {"parse", 2, AR_PURE_CALL_TYPE__PARSE},
    {"build", 2, AR_PURE_CALL_TYPE__BUILD},
    {"if", 3, AR_PURE_CALL_TYPE__IF},
    {"head", 1, AR_PURE_CALL_TYPE__HEAD},
    {"tail", 1, AR_PURE_CALL_TYPE__TAIL},
    {"append", 2, AR_PURE_CALL_TYPE__APPEND}
};

static void test_pure_call__registry_contains_current_pure_calls(void) {
    printf("Testing pure-call registry contains current expression calls...\n");

    // Given the expected pure expression calls
    const size_t expected_count = sizeof(EXPECTED_PURE_CALLS) / sizeof(EXPECTED_PURE_CALLS[0]);

    // Then the registry should contain exactly those calls
    AR_ASSERT(ar_pure_call__count() == expected_count,
              "Registry should contain exactly the current pure calls");

    // When verifying each expected pure call
    for (size_t i = 0; i < expected_count; i++) {
        // When looking up each pure call by name
        const ar_pure_call_t *ref_call = ar_pure_call__find(EXPECTED_PURE_CALLS[i].name);

        // Then its metadata should match the expected entry
        AR_ASSERT(ref_call != NULL, "Registered pure call should be findable");
        AR_ASSERT(ar_pure_call__is_registered(EXPECTED_PURE_CALLS[i].name),
                  "Pure call name should be classified as registered");
        AR_ASSERT(strcmp(ar_pure_call__get_name(ref_call), EXPECTED_PURE_CALLS[i].name) == 0,
                  "Pure call name should match registry entry");
        AR_ASSERT(ar_pure_call__get_arity(ref_call) == EXPECTED_PURE_CALLS[i].arity,
                  "Pure call arity should match registry entry");
        AR_ASSERT(ar_pure_call__get_type(ref_call) == EXPECTED_PURE_CALLS[i].type,
                  "Pure call type should match registry entry");
    }
}

static void test_pure_call__enumerates_registered_calls_in_order(void) {
    printf("Testing pure-call registry enumeration...\n");

    // Given the expected pure expression calls
    const size_t expected_count = sizeof(EXPECTED_PURE_CALLS) / sizeof(EXPECTED_PURE_CALLS[0]);

    // When enumerating registered pure calls
    for (size_t i = 0; i < expected_count; i++) {
        // When reading each registry entry by index
        const ar_pure_call_t *ref_call = ar_pure_call__get_at(i);

        // Then its metadata should match the expected registry order
        AR_ASSERT(ref_call != NULL, "Registered pure call should be enumerable");
        AR_ASSERT(strcmp(ar_pure_call__get_name(ref_call), EXPECTED_PURE_CALLS[i].name) == 0,
                  "Enumerated pure call name should match registry order");
        AR_ASSERT(ar_pure_call__get_arity(ref_call) == EXPECTED_PURE_CALLS[i].arity,
                  "Enumerated pure call arity should match registry order");
        AR_ASSERT(ar_pure_call__get_type(ref_call) == EXPECTED_PURE_CALLS[i].type,
                  "Enumerated pure call type should match registry order");
    }

    // Then indexes outside the registry should not return metadata
    AR_ASSERT(ar_pure_call__get_at(expected_count) == NULL,
              "Out-of-range pure call index should return NULL");
}

static void test_pure_call__rejects_unknown_and_null_names(void) {
    printf("Testing pure-call registry rejects unknown names...\n");

    // Then effectful and NULL names should not resolve as pure calls
    AR_ASSERT(ar_pure_call__find("send") == NULL,
              "Effectful send should not be registered as a pure call");
    AR_ASSERT(!ar_pure_call__is_registered("send"),
              "Effectful send should not be classified as registered");
    AR_ASSERT(ar_pure_call__find(NULL) == NULL,
              "NULL lookup should return NULL");
    AR_ASSERT(!ar_pure_call__is_registered(NULL),
              "NULL name should not be classified as registered");
    AR_ASSERT(ar_pure_call__get_name(NULL) == NULL,
              "NULL metadata entry should not expose a name");
    AR_ASSERT(ar_pure_call__get_arity(NULL) == 0,
              "NULL metadata entry should expose arity 0");
    AR_ASSERT(ar_pure_call__get_type(NULL) == AR_PURE_CALL_TYPE__UNKNOWN,
              "NULL metadata entry should expose unknown type");
}

int main(void) {
    printf("Starting pure-call metadata tests...\n");

    test_pure_call__registry_contains_current_pure_calls();
    test_pure_call__enumerates_registered_calls_in_order();
    test_pure_call__rejects_unknown_and_null_names();

    printf("All pure-call metadata tests passed!\n");
    return 0;
}
