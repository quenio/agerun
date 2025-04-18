#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declaration */
struct map_s;
typedef struct map_s map_t;

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_MAP
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
        map_t *map_value;
    } data;
} data_t;

/**
 * Create a new data value of the specified type with default value
 * @param type Type of data to create
 * @return Data value of the requested type
 */
data_t ar_data_create(data_type_t type);

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_data_free(data_t *data);

#endif /* AGERUN_DATA_H */
