#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "agerun_map.h"

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
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Data value containing the integer
 */
data_t ar_data_create_integer(int64_t value);

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Data value containing the double
 */
data_t ar_data_create_double(double value);

/**
 * Create a new string data value
 * @param value String value to initialize with (will be copied)
 * @return Data value containing the string
 */
data_t ar_data_create_string(const char *value);

/**
 * Create a new map data value
 * @return Data value containing an empty map
 */
data_t ar_data_create_map(void);

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to destroy
 */
void ar_data_destroy(data_t *data);

/**
 * Get the type of a data structure
 * @param data Pointer to the data to check
 * @return The data type or DATA_INT if data is NULL
 */
data_type_t ar_data_get_type(const data_t *data);

/**
 * Get the integer value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 */
int64_t ar_data_get_integer(const data_t *data);

/**
 * Get the double value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 */
double ar_data_get_double(const data_t *data);

/**
 * Get the string value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 */
const char *ar_data_get_string(const data_t *data);

/**
 * Get the map value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
const map_t *ar_data_get_map(const data_t *data);

#endif /* AGERUN_DATA_H */
