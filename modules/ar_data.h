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
    AR_DATA_TYPE__INTEGER,
    AR_DATA_TYPE__DOUBLE,
    AR_DATA_TYPE__STRING,
    AR_DATA_TYPE__LIST,
    AR_DATA_TYPE__MAP
} ar_data_type_t;

/**
 * Opaque data structure for storing various data types
 */
typedef struct ar_data_s ar_data_t;

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_data__create_integer(int value);

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_data__create_double(double value);

/**
 * Create a new string data value
 * @param ref_value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_data__create_string(const char *ref_value);

/**
 * Create a new list data value
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_data__create_list(void);

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_data__create_map(void);

/**
 * Free resources associated with a data structure and release memory
 * @param own_data Pointer to the data to destroy
 * @note Ownership: Takes ownership of the data parameter.
 */
void ar_data__destroy(ar_data_t *own_data);

/**
 * Take ownership of data
 * @param mut_data The data to take ownership of (mutable reference)
 * @param owner The owner taking the data (typically 'this' pointer)
 * @return true if successful, false if already owned by another
 */
bool ar_data__take_ownership(ar_data_t *mut_data, void *owner);

/**
 * Drop ownership of data
 * @param mut_data The data whose ownership to drop (mutable reference)
 * @param owner The current owner dropping the data
 * @return true if successful, false if not the owner
 * @note To release ownership for destruction, drop with your owner pointer
 */
bool ar_data__drop_ownership(ar_data_t *mut_data, void *owner);

/**
 * Create a shallow copy of data values
 * @param ref_value The data value to copy
 * @return New data instance for primitives and flat containers, NULL for nested containers
 * @note Copies primitives (INTEGER, DOUBLE, STRING) and containers with only primitive elements
 * @note Returns NULL if containers have nested containers (no deep copy)
 * @note Ownership: Returns an owned value that caller must destroy, or NULL if cannot copy
 */
ar_data_t* ar_data__shallow_copy(const ar_data_t *ref_value);

/**
 * Claim ownership of data or create a shallow copy if already owned
 * @param ref_data The data to claim or copy (borrowed reference)
 * @param owner The potential owner trying to claim the data
 * @return Owned data (either claimed original or new copy), or NULL if copy failed
 * @note If data is unowned, claims it and returns the same pointer
 * @note If data is owned by someone else, returns a shallow copy
 * @note Returns NULL only if shallow copy fails (nested containers)
 * @note Ownership: Returns owned value that caller must destroy
 */
ar_data_t* ar_data__claim_or_copy(ar_data_t *ref_data, void *owner);

/**
 * Safely discard data if we can take ownership of it
 * @param ref_data The data to potentially discard (borrowed reference)
 * @param owner The potential owner trying to discard the data
 * @note If data is unowned, takes ownership and destroys it
 * @note If data is owned by someone else, does nothing (safe)
 * @note Ownership: Does not transfer ownership to caller
 */
void ar_data__destroy_if_owned(ar_data_t *ref_data, void *owner);

/**
 * Get the type of a data structure
 * @param ref_data Pointer to the data to check
 * @return The data type or AR_DATA_TYPE__INTEGER if data is NULL
 * @note Ownership: Does not take ownership of the data parameter.
 */
ar_data_type_t ar_data__get_type(const ar_data_t *ref_data);

/**
 * Check if a data value is a primitive type
 * @param ref_data The data value to check
 * @return true if the data is INTEGER, DOUBLE, or STRING; false otherwise
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__is_primitive_type(const ar_data_t *ref_data);

/**
 * Check if a map contains only primitive values
 * @param ref_data The map data to check
 * @return true if the map contains only INTEGER, DOUBLE, or STRING values; false otherwise
 * @note Returns false if ref_data is NULL or not a map
 * @note Returns true for empty maps
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__map_contains_only_primitives(const ar_data_t *ref_data);

/**
 * Check if a list contains only primitive values
 * @param ref_data The list data to check
 * @return true if the list contains only INTEGER, DOUBLE, or STRING values; false otherwise
 * @note Returns false if ref_data is NULL or not a list
 * @note Returns true for empty lists
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__list_contains_only_primitives(const ar_data_t *ref_data);

/**
 * Get the integer value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 * @note Ownership: Does not take ownership of the data parameter.
 */
int ar_data__get_integer(const ar_data_t *ref_data);

/**
 * Get the double value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 * @note Ownership: Does not take ownership of the data parameter.
 */
double ar_data__get_double(const ar_data_t *ref_data);

/**
 * Get the string value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 * @note Ownership: Does not take ownership of the data parameter.
 */
const char *ar_data__get_string(const ar_data_t *ref_data);

/**
 * NOTE: Removed ar_data_get_map and ar_data_get_map_mutable
 * These functions are no longer necessary since we have ar_data_get_map_data
 * and other accessor functions. Modules outside data.c should never
 * have direct access to ar_map_t instances.
 */

/**
 * Get an integer value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 * @note Ownership: Does not take ownership of the parameters.
 */
int ar_data__get_map_integer(const ar_data_t *ref_data, const char *ref_key);

/**
 * Get a double value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 * @note Ownership: Does not take ownership of the parameters.
 */
double ar_data__get_map_double(const ar_data_t *ref_data, const char *ref_key);

/**
 * Get a string value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 * @note Ownership: Does not take ownership of the parameters.
 */
const char *ar_data__get_map_string(const ar_data_t *ref_data, const char *ref_key);

/**
 * Get a data value from a map data structure by key or path
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 * @note Ownership: Does not take ownership of the parameters. Returns a borrowed reference.
 */
ar_data_t *ar_data__get_map_data(const ar_data_t *ref_data, const char *ref_key);

/**
 * Set an integer value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_integer(ar_data_t *mut_data, const char *ref_key, int value);

/**
 * Set a double value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_double(ar_data_t *mut_data, const char *ref_key, double value);

/**
 * Set a string value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param ref_value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_string(ar_data_t *mut_data, const char *ref_key, const char *ref_value);

/**
 * Set a data value in a map data structure by key or path
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param own_value The data value to store (ownership is transferred)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__set_map_data(ar_data_t *mut_data, const char *ref_key, ar_data_t *own_value);

/**
 * Add an integer value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_integer(ar_data_t *mut_data, int value);

/**
 * Add a double value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_double(ar_data_t *mut_data, double value);

/**
 * Add a string value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_string(ar_data_t *mut_data, const char *ref_value);

/**
 * Add a data value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__list_add_first_data(ar_data_t *mut_data, ar_data_t *own_value);

/**
 * Add an integer value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_integer(ar_data_t *mut_data, int value);

/**
 * Add a double value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_double(ar_data_t *mut_data, double value);

/**
 * Add a string value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_string(ar_data_t *mut_data, const char *ref_value);

/**
 * Add a data value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__list_add_last_data(ar_data_t *mut_data, ar_data_t *own_value);

/**
 * Remove and return the first data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t *ar_data__list_remove_first(ar_data_t *mut_data);

/**
 * Remove and return the last data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t *ar_data__list_remove_last(ar_data_t *mut_data);

/**
 * Remove and return the first integer value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or first item not an integer
 * @note This function also removes and frees the data structure containing the integer
 * @note Ownership: Does not transfer any ownership for the return value.
 */
int ar_data__list_remove_first_integer(ar_data_t *mut_data);

/**
 * Remove and return the first double value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or first item not a double
 * @note This function also removes and frees the data structure containing the double
 * @note Ownership: Does not transfer any ownership for the return value.
 */
double ar_data__list_remove_first_double(ar_data_t *mut_data);

/**
 * Remove and return the first string value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or first item not a string
 * @note This function also removes and frees the data structure containing the string reference
 * @note Ownership: Returns an owned string that caller must free.
 */
char *ar_data__list_remove_first_string(ar_data_t *mut_data);

/**
 * Remove and return the last integer value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or last item not an integer
 * @note This function also removes and frees the data structure containing the integer
 * @note Ownership: Does not transfer any ownership for the return value.
 */
int ar_data__list_remove_last_integer(ar_data_t *mut_data);

/**
 * Remove and return the last double value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or last item not a double
 * @note This function also removes and frees the data structure containing the double
 * @note Ownership: Does not transfer any ownership for the return value.
 */
double ar_data__list_remove_last_double(ar_data_t *mut_data);

/**
 * Remove and return the last string value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or last item not a string
 * @note This function also removes and frees the data structure containing the string reference
 * @note Ownership: Returns an owned string that caller must free.
 */
char *ar_data__list_remove_last_string(ar_data_t *mut_data);

/**
 * Get the first data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The first data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
ar_data_t *ar_data__list_first(const ar_data_t *ref_data);

/**
 * Get the last data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The last data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
ar_data_t *ar_data__list_last(const ar_data_t *ref_data);

/**
 * Get the number of items in a list data structure
 * @param ref_data Pointer to the list data
 * @return The number of items, or 0 if data is NULL or not a list
 * @note Ownership: Does not take ownership of the data parameter.
 */
size_t ar_data__list_count(const ar_data_t *ref_data);

/**
 * Get all keys from a map data structure
 * @param ref_data Pointer to the map data to retrieve keys from
 * @return A list containing string data values for each key, or NULL if data is NULL or not a map
 * @note Ownership: Returns an owned list that caller must destroy.
 *       The returned list contains string data values (not raw strings).
 *       An empty map returns an empty list (not NULL).
 */
ar_data_t* ar_data__get_map_keys(const ar_data_t *ref_data);

/**
 * Set a value in a map if the path's root segment matches the expected root.
 * Only stores the value if the path starts with the expected root segment.
 * 
 * @param mut_map The map to update
 * @param ref_expected_root The required root segment (e.g., "memory")
 * @param ref_full_path The full path (e.g., "memory.x" or NULL)
 * @param own_value The value to store (ownership transferred only on success)
 * @return true if value was stored, false if path was invalid/NULL/wrong root
 * @note Ownership: Takes ownership of own_value ONLY if returning true
 * @example ar_data__set_map_data_if_root_matched(map, "memory", "memory.x", value) → stores at "x", returns true
 * @example ar_data__set_map_data_if_root_matched(map, "memory", "context.x", value) → returns false
 * @example ar_data__set_map_data_if_root_matched(map, "memory", NULL, value) → returns false
 */
bool ar_data__set_map_data_if_root_matched(
    ar_data_t *mut_map,
    const char *ref_expected_root,
    const char *ref_full_path,
    ar_data_t *own_value
);

#endif /* AGERUN_DATA_H */
