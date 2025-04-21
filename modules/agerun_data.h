#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "agerun_map.h"
#include "agerun_list.h"

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INTEGER,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_LIST,
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
 * Create a new list data value
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_list(void);

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
 * NOTE: Removed ar_data_get_map and ar_data_get_map_mutable
 * These functions are no longer necessary since we have ar_data_get_map_data
 * and other accessor functions. Modules outside data.c should never
 * have direct access to map_t instances.
 */

/**
 * Get an integer value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 */
int ar_data_get_map_integer(const data_t *data, const char *key);

/**
 * Get a double value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 */
double ar_data_get_map_double(const data_t *data, const char *key);

/**
 * Get a string value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 */
const char *ar_data_get_map_string(const data_t *data, const char *key);

/**
 * Get a data value from a map data structure by key or path
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 */
data_t *ar_data_get_map_data(const data_t *data, const char *key);

/**
 * Set an integer value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_integer(data_t *data, const char *key, int value);

/**
 * Set a double value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_double(data_t *data, const char *key, double value);

/**
 * Set a string value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_string(data_t *data, const char *key, const char *value);

/**
 * Set a data value in a map data structure by key or path
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The data value to store (ownership is transferred)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_data(data_t *data, const char *key, data_t *value);

/**
 * Add an integer value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_integer(data_t *data, int value);

/**
 * Add a double value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_double(data_t *data, double value);

/**
 * Add a string value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_string(data_t *data, const char *value);

/**
 * Add a data value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_data(data_t *data, data_t *value);

/**
 * Add an integer value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_integer(data_t *data, int value);

/**
 * Add a double value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_double(data_t *data, double value);

/**
 * Add a string value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_string(data_t *data, const char *value);

/**
 * Add a data value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_data(data_t *data, data_t *value);

/**
 * Remove and return the first data value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_remove_first(data_t *data);

/**
 * Remove and return the last data value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_remove_last(data_t *data);

/**
 * Get the first data value from a list data structure (without removing it)
 * @param data Pointer to the list data
 * @return The first data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_first(const data_t *data);

/**
 * Get the last data value from a list data structure (without removing it)
 * @param data Pointer to the list data
 * @return The last data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_last(const data_t *data);

/**
 * Get the number of items in a list data structure
 * @param data Pointer to the list data
 * @return The number of items, or 0 if data is NULL or not a list
 */
size_t ar_data_list_count(const data_t *data);

#endif /* AGERUN_DATA_H */
