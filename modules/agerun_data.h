#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "agerun_map.h"

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INTEGER,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_MAP
} data_type_t;

/**
 * Opaque data structure for storing various data types
 */
typedef struct data_s data_t;

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_integer(int value);

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_double(double value);

/**
 * Create a new string data value
 * @param value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_string(const char *value);

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_map(void);

/**
 * Free resources associated with a data structure and release memory
 * @param data Pointer to the data to destroy
 */
void ar_data_destroy(data_t *data);

/**
 * Get the type of a data structure
 * @param data Pointer to the data to check
 * @return The data type or DATA_INTEGER if data is NULL
 */
data_type_t ar_data_get_type(const data_t *data);

/**
 * Get the integer value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 */
int ar_data_get_integer(const data_t *data);

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
 * Get the map value from a data structure (read-only)
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
const map_t *ar_data_get_map(const data_t *data);

/**
 * Get a mutable map value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
map_t *ar_data_get_map_mutable(data_t *data);

/**
 * Get an integer value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 */
int ar_data_get_sub_integer(const data_t *data, const char *key);

/**
 * Get a double value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 */
double ar_data_get_sub_double(const data_t *data, const char *key);

/**
 * Get a string value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 */
const char *ar_data_get_sub_string(const data_t *data, const char *key);


/**
 * Set an integer value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key to set in the map
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_sub_integer(data_t *data, const char *key, int value);

/**
 * Set a double value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key to set in the map
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_sub_double(data_t *data, const char *key, double value);

/**
 * Set a string value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key to set in the map
 * @param value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_sub_string(data_t *data, const char *key, const char *value);

#endif /* AGERUN_DATA_H */
