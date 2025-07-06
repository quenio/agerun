#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "ar_map.h"
#include "ar_list.h"

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INTEGER,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_LIST,
    DATA_MAP
} ar_data_type_t;

/**
 * Opaque data structure for storing various data types
 */
typedef struct data_s data_t;

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar_data__create_integer(int value);

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar_data__create_double(double value);

/**
 * Create a new string data value
 * @param ref_value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar_data__create_string(const char *ref_value);

/**
 * Create a new list data value
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar_data__create_list(void);

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t* ar_data__create_map(void);

/**
 * Free resources associated with a data structure and release memory
 * @param own_data Pointer to the data to destroy
 * @note Ownership: Takes ownership of the data parameter.
 */
void ar_data__destroy(data_t *own_data);

/**
 * Claim or accept ownership of data
 * @param mut_data The data to claim (mutable reference)
 * @param owner The owner claiming the data (typically 'this' pointer)
 * @return true if successful, false if already owned by another
 */
bool ar_data__hold_ownership(data_t *mut_data, void *owner);

/**
 * Transfer ownership of data
 * @param mut_data The data to transfer (mutable reference)
 * @param owner The current owner transferring the data
 * @return true if successful, false if not the owner
 * @note To release ownership for destruction, transfer with your owner pointer
 */
bool ar_data__transfer_ownership(data_t *mut_data, void *owner);

/**
 * Create a shallow copy of data values
 * @param ref_value The data value to copy
 * @return New data instance for primitives and flat containers, NULL for nested containers
 * @note Copies primitives (INTEGER, DOUBLE, STRING) and containers with only primitive elements
 * @note Returns NULL if containers have nested containers (no deep copy)
 * @note Ownership: Returns an owned value that caller must destroy, or NULL if cannot copy
 */
data_t* ar_data__shallow_copy(const data_t *ref_value);

/**
 * Get the type of a data structure
 * @param ref_data Pointer to the data to check
 * @return The data type or DATA_INTEGER if data is NULL
 * @note Ownership: Does not take ownership of the data parameter.
 */
ar_data_type_t ar_data__get_type(const data_t *ref_data);

/**
 * Check if a data value is a primitive type
 * @param ref_data The data value to check
 * @return true if the data is INTEGER, DOUBLE, or STRING; false otherwise
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__is_primitive_type(const data_t *ref_data);

/**
 * Check if a map contains only primitive values
 * @param ref_data The map data to check
 * @return true if the map contains only INTEGER, DOUBLE, or STRING values; false otherwise
 * @note Returns false if ref_data is NULL or not a map
 * @note Returns true for empty maps
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__map_contains_only_primitives(const data_t *ref_data);

/**
 * Check if a list contains only primitive values
 * @param ref_data The list data to check
 * @return true if the list contains only INTEGER, DOUBLE, or STRING values; false otherwise
 * @note Returns false if ref_data is NULL or not a list
 * @note Returns true for empty lists
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__list_contains_only_primitives(const data_t *ref_data);

/**
 * Get the integer value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 * @note Ownership: Does not take ownership of the data parameter.
 */
int ar_data__get_integer(const data_t *ref_data);

/**
 * Get the double value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 * @note Ownership: Does not take ownership of the data parameter.
 */
double ar_data__get_double(const data_t *ref_data);

/**
 * Get the string value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 * @note Ownership: Does not take ownership of the data parameter.
 */
const char *ar_data__get_string(const data_t *ref_data);

/**
 * NOTE: Removed ar_data_get_map and ar_data_get_map_mutable
 * These functions are no longer necessary since we have ar_data_get_map_data
 * and other accessor functions. Modules outside data.c should never
 * have direct access to map_t instances.
 */

/**
 * Get an integer value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 * @note Ownership: Does not take ownership of the parameters.
 */
int ar_data__get_map_integer(const data_t *ref_data, const char *ref_key);

/**
 * Get a double value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 * @note Ownership: Does not take ownership of the parameters.
 */
double ar_data__get_map_double(const data_t *ref_data, const char *ref_key);

/**
 * Get a string value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 * @note Ownership: Does not take ownership of the parameters.
 */
const char *ar_data__get_map_string(const data_t *ref_data, const char *ref_key);

/**
 * Get a data value from a map data structure by key or path
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 * @note Ownership: Does not take ownership of the parameters. Returns a borrowed reference.
 */
data_t *ar_data__get_map_data(const data_t *ref_data, const char *ref_key);

/**
 * Set an integer value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_integer(data_t *mut_data, const char *ref_key, int value);

/**
 * Set a double value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_double(data_t *mut_data, const char *ref_key, double value);

/**
 * Set a string value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param ref_value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_string(data_t *mut_data, const char *ref_key, const char *ref_value);

/**
 * Set a data value in a map data structure by key or path
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param own_value The data value to store (ownership is transferred)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__set_map_data(data_t *mut_data, const char *ref_key, data_t *own_value);

/**
 * Add an integer value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_integer(data_t *mut_data, int value);

/**
 * Add a double value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_double(data_t *mut_data, double value);

/**
 * Add a string value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_string(data_t *mut_data, const char *ref_value);

/**
 * Add a data value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__list_add_first_data(data_t *mut_data, data_t *own_value);

/**
 * Add an integer value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_integer(data_t *mut_data, int value);

/**
 * Add a double value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_double(data_t *mut_data, double value);

/**
 * Add a string value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_string(data_t *mut_data, const char *ref_value);

/**
 * Add a data value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__list_add_last_data(data_t *mut_data, data_t *own_value);

/**
 * Remove and return the first data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t *ar_data__list_remove_first(data_t *mut_data);

/**
 * Remove and return the last data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
data_t *ar_data__list_remove_last(data_t *mut_data);

/**
 * Remove and return the first integer value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or first item not an integer
 * @note This function also removes and frees the data structure containing the integer
 * @note Ownership: Does not transfer any ownership for the return value.
 */
int ar_data__list_remove_first_integer(data_t *mut_data);

/**
 * Remove and return the first double value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or first item not a double
 * @note This function also removes and frees the data structure containing the double
 * @note Ownership: Does not transfer any ownership for the return value.
 */
double ar_data__list_remove_first_double(data_t *mut_data);

/**
 * Remove and return the first string value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or first item not a string
 * @note This function also removes and frees the data structure containing the string reference
 * @note Ownership: Returns an owned string that caller must free.
 */
char *ar_data__list_remove_first_string(data_t *mut_data);

/**
 * Remove and return the last integer value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or last item not an integer
 * @note This function also removes and frees the data structure containing the integer
 * @note Ownership: Does not transfer any ownership for the return value.
 */
int ar_data__list_remove_last_integer(data_t *mut_data);

/**
 * Remove and return the last double value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or last item not a double
 * @note This function also removes and frees the data structure containing the double
 * @note Ownership: Does not transfer any ownership for the return value.
 */
double ar_data__list_remove_last_double(data_t *mut_data);

/**
 * Remove and return the last string value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or last item not a string
 * @note This function also removes and frees the data structure containing the string reference
 * @note Ownership: Returns an owned string that caller must free.
 */
char *ar_data__list_remove_last_string(data_t *mut_data);

/**
 * Get the first data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The first data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
data_t *ar_data__list_first(const data_t *ref_data);

/**
 * Get the last data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The last data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
data_t *ar_data__list_last(const data_t *ref_data);

/**
 * Get the number of items in a list data structure
 * @param ref_data Pointer to the list data
 * @return The number of items, or 0 if data is NULL or not a list
 * @note Ownership: Does not take ownership of the data parameter.
 */
size_t ar_data__list_count(const data_t *ref_data);

/**
 * Get all keys from a map data structure
 * @param ref_data Pointer to the map data to retrieve keys from
 * @return A list containing string data values for each key, or NULL if data is NULL or not a map
 * @note Ownership: Returns an owned list that caller must destroy.
 *       The returned list contains string data values (not raw strings).
 *       An empty map returns an empty list (not NULL).
 */
data_t* ar_data__get_map_keys(const data_t *ref_data);

#endif /* AGERUN_DATA_H */
