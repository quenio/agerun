#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>

/**
 * Value type enumeration
 */
typedef enum {
    VALUE_INT,
    VALUE_DOUBLE,
    VALUE_STRING
} value_type_t;

/**
 * Value structure for storing various data types
 */
typedef struct value_s {
    value_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
    } data;
} value_t;

/**
 * Free resources associated with a value
 * @param value Pointer to the value to free
 */
void ar_free_value(value_t *value);

#endif /* AGERUN_DATA_H */
