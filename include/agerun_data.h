#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING
} data_type_t;

/**
 * Data structure for storing various data types
 */
typedef struct data_s {
    data_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
    } data;
} data_t;

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_free_data(data_t *data);

#endif /* AGERUN_DATA_H */
