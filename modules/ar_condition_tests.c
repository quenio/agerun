#include <stdio.h>
#include "ar_assert.h"
#include "ar_condition.h"
#include "ar_data.h"

static void test_condition__integer_zero_is_false(void) {
    printf("Testing condition integer zero is false...\n");

    // Given an integer zero value
    ar_data_t *own_value = ar_data__create_integer(0);
    AR_ASSERT(own_value != NULL, "Integer value creation should succeed");

    // Then it should be false under condition truthiness
    AR_ASSERT(ar_condition__is_true(own_value) == false,
              "Integer zero should be false");

    // Cleanup
    ar_data__destroy(own_value);
}

static void test_condition__nonzero_integer_is_true(void) {
    printf("Testing condition nonzero integer is true...\n");

    // Given a nonzero integer value
    ar_data_t *own_value = ar_data__create_integer(7);
    AR_ASSERT(own_value != NULL, "Integer value creation should succeed");

    // Then it should be true under condition truthiness
    AR_ASSERT(ar_condition__is_true(own_value) == true,
              "Nonzero integer should be true");

    // Cleanup
    ar_data__destroy(own_value);
}

static void test_condition__non_integer_is_false(void) {
    printf("Testing condition non-integer is false...\n");

    // Given a non-integer value
    ar_data_t *own_value = ar_data__create_string("truthy-looking");
    AR_ASSERT(own_value != NULL, "String value creation should succeed");

    // Then it should be false under condition truthiness
    AR_ASSERT(ar_condition__is_true(own_value) == false,
              "Non-integer value should be false");

    // Cleanup
    ar_data__destroy(own_value);
}

static void test_condition__null_is_false(void) {
    printf("Testing condition NULL is false...\n");

    // Then a missing value should be false under condition truthiness
    AR_ASSERT(ar_condition__is_true(NULL) == false,
              "Missing value should be false");
}

int main(void) {
    printf("Running condition tests...\n\n");

    test_condition__integer_zero_is_false();
    test_condition__nonzero_integer_is_true();
    test_condition__non_integer_is_false();
    test_condition__null_is_false();

    printf("\nAll condition tests passed!\n");
    return 0;
}
