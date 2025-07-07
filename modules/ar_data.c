#include "ar_data.h"
#include "ar_string.h"
#include "ar_list.h"
#include "ar_assert.h"
#include "ar_heap.h"
#include "ar_path.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Data structure for storing various data types
 */
struct ar_data_s {
    ar_data_type_t type;
    union {
        int int_value;         // Primitive type, no prefix needed
        double double_value;   // Primitive type, no prefix needed
        char *own_string;      // Owned string that ar_data_t owns and must free
        ar_list_t *own_list;      // Owned list that ar_data_t owns and must free
        ar_map_t *own_map;        // Owned map that ar_data_t owns and must free
    } data;
    ar_list_t *own_keys;  // List of keys that belong to this data's map (only used for AR_DATA_TYPE__MAP type)
    void *owner;       // NULL = unowned, non-NULL = owned
};

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
ar_data_t* ar_data__create_integer(int value) {
    ar_data_t* data = (ar_data_t*)AR__HEAP__MALLOC(sizeof(ar_data_t), "Integer data");
    if (!data) {
        return NULL;
    }
    
    data->type = AR_DATA_TYPE__INTEGER;
    data->data.int_value = value;
    data->own_keys = NULL;
    data->owner = NULL;  // Start unowned
    return data;
}

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
ar_data_t* ar_data__create_double(double value) {
    ar_data_t* data = (ar_data_t*)AR__HEAP__MALLOC(sizeof(ar_data_t), "Integer data");
    if (!data) {
        return NULL;
    }
    
    data->type = AR_DATA_TYPE__DOUBLE;
    data->data.double_value = value;
    data->own_keys = NULL;
    data->owner = NULL;  // Start unowned
    return data;
}

/**
 * Create a new string data value
 * @param ref_value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_data__create_string(const char *ref_value) {
    ar_data_t* own_data = AR__HEAP__MALLOC(sizeof(ar_data_t), "String data structure");
    if (!own_data) {
        return NULL;
    }
    
    own_data->type = AR_DATA_TYPE__STRING;
    own_data->data.own_string = ref_value ? AR__HEAP__STRDUP(ref_value, "String data value") : NULL;
    if (ref_value && !own_data->data.own_string) {
        AR__HEAP__FREE(own_data);
        return NULL;
    }
    
    own_data->own_keys = NULL;
    own_data->owner = NULL;  // Start unowned
    return own_data; // Ownership transferred to caller
}

/**
 * Create a new list data value
 * @return Pointer to the new data, or NULL on failure
 */
ar_data_t* ar_data__create_list(void) {
    ar_data_t* data = (ar_data_t*)AR__HEAP__MALLOC(sizeof(ar_data_t), "Integer data");
    if (!data) {
        return NULL;
    }
    
    data->type = AR_DATA_TYPE__LIST;
    data->data.own_list = ar_list__create();
    if (!data->data.own_list) {
        AR__HEAP__FREE(data);
        return NULL;
    }
    
    data->own_keys = NULL;
    data->owner = NULL;  // Start unowned
    return data;
}

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 */
ar_data_t* ar_data__create_map(void) {
    ar_data_t* data = (ar_data_t*)AR__HEAP__MALLOC(sizeof(ar_data_t), "Integer data");
    if (!data) {
        return NULL;
    }
    
    data->type = AR_DATA_TYPE__MAP;
    data->data.own_map = ar_map__create();
    if (!data->data.own_map) {
        AR__HEAP__FREE(data);
        return NULL;
    }
    
    // Create a list to track keys
    data->own_keys = ar_list__create();
    if (!data->own_keys) {
        ar_map__destroy(data->data.own_map);
        AR__HEAP__FREE(data);
        return NULL;
    }
    
    data->owner = NULL;  // Start unowned
    return data;
}

// This function has been replaced by ar_map__refs

/**
 * Free resources associated with a data structure and release memory
 * @param own_data Pointer to the data to destroy
 * @note Ownership: Takes ownership of the own_data parameter.
 *       Ownership is fully consumed by this function.
 */
void ar_data__destroy(ar_data_t *own_data) {
    if (!own_data) return;
    
    // Can only destroy unowned data
    if (own_data->owner != NULL) {
        fprintf(stderr, "ar_data__destroy: Cannot destroy owned data\n");
        AR_ASSERT(false, "Cannot destroy owned data");
        return;
    }
    
    if (own_data->type == AR_DATA_TYPE__STRING && own_data->data.own_string) {
        AR__HEAP__FREE(own_data->data.own_string);
        own_data->data.own_string = NULL;
    } else if (own_data->type == AR_DATA_TYPE__LIST && own_data->data.own_list) {
        // For lists, we need to:
        // 1. Get all data values stored in the list (for later cleanup)
        // 2. Free the list structure itself
        // 3. Then free all the data values
        
        // Get all data values for later cleanup
        void **own_items = ar_list__items(own_data->data.own_list);
        size_t item_count = ar_list__count(own_data->data.own_list);
        
        // Destroy the list structure first
        ar_list__destroy(own_data->data.own_list);
        own_data->data.own_list = NULL;
        
        // Free all data values
        if (own_items) {
            for (size_t i = 0; i < item_count; i++) {
                ar_data_t *item = (ar_data_t*)own_items[i];
                if (item) {
                    // Only transfer ownership if we actually own it
                    if (item->owner == own_data) {
                        ar_data__transfer_ownership(item, own_data);
                    }
                    ar_data__destroy(item); // Ownership transferred to ar_data__destroy
                }
            }
            AR__HEAP__FREE(own_items); // Free the array itself
            own_items = NULL; // Mark as freed
        }
    } else if (own_data->type == AR_DATA_TYPE__MAP && own_data->data.own_map) {
        // For maps, we need to:
        // 1. Get all data values stored in the map (for later cleanup)
        // 2. Free the map structure itself
        // 3. Free all keys tracked in our list
        // 4. Free the list itself
        // 5. Then free all the data values
        
        // Get all data values for later cleanup
        void **own_refs = ar_map__refs(own_data->data.own_map);
        size_t ref_count = ar_map__count(own_data->data.own_map);
        
        // Destroy the map structure first
        ar_map__destroy(own_data->data.own_map);
        own_data->data.own_map = NULL;
        
        // Free all tracked keys
        if (own_data->own_keys) {
            void **own_key_ptrs = ar_list__items(own_data->own_keys);
            size_t key_count = ar_list__count(own_data->own_keys);
            
            if (own_key_ptrs) {
                for (size_t i = 0; i < key_count; i++) {
                    AR__HEAP__FREE(own_key_ptrs[i]); // Free each key string
                }
                AR__HEAP__FREE(own_key_ptrs); // Free the array itself
                own_key_ptrs = NULL; // Mark as freed
            }
            
            // Destroy the key tracking list
            ar_list__destroy(own_data->own_keys);
            own_data->own_keys = NULL;
        }
        
        // Free all data values
        if (own_refs) {
            for (size_t i = 0; i < ref_count; i++) {
                ar_data_t *item = (ar_data_t*)own_refs[i];
                if (item) {
                    // Only transfer ownership if we actually own it
                    if (item->owner == own_data) {
                        ar_data__transfer_ownership(item, own_data);
                    }
                    ar_data__destroy(item); // Ownership transferred to ar_data__destroy
                }
            }
            AR__HEAP__FREE(own_refs); // Free the array itself
            own_refs = NULL; // Mark as freed
        }
    }
    
    AR__HEAP__FREE(own_data);
    // Ownership consumed completely
}

/**
 * Claim or accept ownership of data
 * @param mut_data The data to claim (mutable reference)
 * @param owner The owner claiming the data (typically 'this' pointer)
 * @return true if successful, false if already owned by another
 */
bool ar_data__hold_ownership(ar_data_t *mut_data, void *owner) {
    if (!mut_data || !owner) return false;
    
    if (mut_data->owner == NULL || mut_data->owner == owner) {
        mut_data->owner = owner;
        return true;
    }
    return false;
}

/**
 * Transfer ownership of data
 * @param mut_data The data to transfer (mutable reference)
 * @param owner The current owner transferring the data
 * @return true if successful, false if not the owner
 * @note To release ownership for destruction, transfer with your owner pointer
 */
bool ar_data__transfer_ownership(ar_data_t *mut_data, void *owner) {
    if (!mut_data || !owner) return false;
    
    if (mut_data->owner == owner) {
        mut_data->owner = NULL;  // Release for next holder
        return true;
    }
    return false;
}

/* Helper function to shallow copy a map with only primitive values */
static ar_data_t* _shallow_copy_map(const ar_data_t *ref_map) {
    // Check if map contains only primitives
    if (!ar_data__map_contains_only_primitives(ref_map)) {
        return NULL;
    }
    
    // Create a new map
    ar_data_t *own_new_map = ar_data__create_map();
    if (!own_new_map) return NULL;
    
    // Get all keys from the original map
    ar_data_t *own_keys = ar_data__get_map_keys(ref_map);
    if (!own_keys) {
        return own_new_map; // Empty map
    }
    
    // Copy all key-value pairs
    size_t key_count = ar_data__list_count(own_keys);
    for (size_t i = 0; i < key_count; i++) {
        ar_data_t *ref_key_data = ar_data__list_first(own_keys);
        if (!ref_key_data) break;
        
        const char *ref_key = ar_data__get_string(ref_key_data);
        if (ref_key) {
            ar_data_t *ref_orig_value = ar_data__get_map_data(ref_map, ref_key);
            if (ref_orig_value) {
                ar_data_t *own_copy_value = ar_data__shallow_copy(ref_orig_value);
                if (own_copy_value) {
                    bool success = ar_data__set_map_data(own_new_map, ref_key, own_copy_value);
                    if (!success) {
                        ar_data__destroy(own_copy_value);
                    }
                }
            }
        }
        
        // Remove and destroy the processed key
        ar_data_t *own_removed_key = ar_data__list_remove_first(own_keys);
        ar_data__destroy(own_removed_key);
    }
    
    ar_data__destroy(own_keys);
    return own_new_map;
}

/* Helper function to shallow copy a list with only primitive values */
static ar_data_t* _shallow_copy_list(const ar_data_t *ref_list) {
    // Check if list contains only primitives
    if (!ar_data__list_contains_only_primitives(ref_list)) {
        return NULL;
    }
    
    // Create a new list
    ar_data_t *own_new_list = ar_data__create_list();
    if (!own_new_list) return NULL;
    
    // Get list items
    ar_list_t *ref_internal_list = ref_list->data.own_list;
    void **items = ar_list__items(ref_internal_list);
    size_t count = ar_list__count(ref_internal_list);
    
    if (!items || count == 0) {
        return own_new_list; // Empty list
    }
    
    // Copy all items
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_item = (ar_data_t*)items[i];
        if (ref_item) {
            ar_data_t *own_copy_item = ar_data__shallow_copy(ref_item);
            if (own_copy_item) {
                bool success = ar_data__list_add_last_data(own_new_list, own_copy_item);
                if (!success) {
                    ar_data__destroy(own_copy_item);
                }
            }
        }
    }
    
    AR__HEAP__FREE(items);
    return own_new_list;
}

/**
 * Create a shallow copy of data values
 * @param ref_value The data value to copy
 * @return New data instance for primitives and flat containers, NULL for nested containers
 * @note Copies primitives (INTEGER, DOUBLE, STRING) and containers with only primitive elements
 * @note Returns NULL if containers have nested containers (no deep copy)
 * @note Ownership: Returns an owned value that caller must destroy, or NULL if cannot copy
 */
ar_data_t* ar_data__shallow_copy(const ar_data_t *ref_value) {
    if (!ref_value) {
        return NULL;
    }
    
    ar_data_type_t type = ar_data__get_type(ref_value);
    
    switch (type) {
        case AR_DATA_TYPE__INTEGER:
            return ar_data__create_integer(ar_data__get_integer(ref_value));
            
        case AR_DATA_TYPE__DOUBLE:
            return ar_data__create_double(ar_data__get_double(ref_value));
            
        case AR_DATA_TYPE__STRING:
            return ar_data__create_string(ar_data__get_string(ref_value));
            
        case AR_DATA_TYPE__MAP:
            return _shallow_copy_map(ref_value);
            
        case AR_DATA_TYPE__LIST:
            return _shallow_copy_list(ref_value);
            
        default:
            return NULL;
    }
}

/**
 * Get the type of a data structure
 * @param ref_data Pointer to the data to check
 * @return The data type or AR_DATA_TYPE__INTEGER if data is NULL
 * @note Ownership: Does not take ownership of the data parameter.
 */
ar_data_type_t ar_data__get_type(const ar_data_t *ref_data) {
    if (!ref_data) {
        return AR_DATA_TYPE__INTEGER; // Default to int if NULL
    }
    return ref_data->type;
}

/**
 * Check if a data value is a primitive type
 * @param ref_data The data value to check
 * @return true if the data is INTEGER, DOUBLE, or STRING; false otherwise
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__is_primitive_type(const ar_data_t *ref_data) {
    if (!ref_data) {
        return false;
    }
    
    ar_data_type_t type = ar_data__get_type(ref_data);
    return (type == AR_DATA_TYPE__INTEGER || type == AR_DATA_TYPE__DOUBLE || type == AR_DATA_TYPE__STRING);
}

/**
 * Check if a map contains only primitive values
 * @param ref_data The map data to check
 * @return true if the map contains only INTEGER, DOUBLE, or STRING values; false otherwise
 * @note Returns false if ref_data is NULL or not a map
 * @note Returns true for empty maps
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__map_contains_only_primitives(const ar_data_t *ref_data) {
    if (!ref_data || ar_data__get_type(ref_data) != AR_DATA_TYPE__MAP) {
        return false;
    }
    
    // Get all keys to iterate through map
    ar_data_t *own_keys = ar_data__get_map_keys(ref_data);
    if (!own_keys) {
        return true; // Empty map
    }
    
    bool all_primitives = true;
    size_t count = ar_data__list_count(own_keys);
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_key_data = ar_data__list_first(own_keys);
        if (!ref_key_data) break;
        
        const char *ref_key = ar_data__get_string(ref_key_data);
        if (!ref_key) {
            ar_data_t *own_removed = ar_data__list_remove_first(own_keys);
            ar_data__destroy(own_removed);
            continue;
        }
        
        // Get the value for this key
        ar_data_t *ref_value = ar_data__get_map_data(ref_data, ref_key);
        if (ref_value && !ar_data__is_primitive_type(ref_value)) {
            all_primitives = false;
            break;
        }
        
        // Remove and destroy the processed key
        ar_data_t *own_removed_key = ar_data__list_remove_first(own_keys);
        ar_data__destroy(own_removed_key);
    }
    
    // Clean up remaining keys if we broke out early
    while (ar_data__list_count(own_keys) > 0) {
        ar_data_t *own_key = ar_data__list_remove_first(own_keys);
        ar_data__destroy(own_key);
    }
    
    ar_data__destroy(own_keys);
    return all_primitives;
}

/**
 * Check if a list contains only primitive values
 * @param ref_data The list data to check
 * @return true if the list contains only INTEGER, DOUBLE, or STRING values; false otherwise
 * @note Returns false if ref_data is NULL or not a list
 * @note Returns true for empty lists
 * @note Ownership: Does not take ownership of the data parameter.
 */
bool ar_data__list_contains_only_primitives(const ar_data_t *ref_data) {
    if (!ref_data || ar_data__get_type(ref_data) != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Get list items
    void **items = ar_list__items(ref_data->data.own_list);
    size_t count = ar_list__count(ref_data->data.own_list);
    
    if (!items || count == 0) {
        return true; // Empty list
    }
    
    bool all_primitives = true;
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_item = (ar_data_t*)items[i];
        if (ref_item && !ar_data__is_primitive_type(ref_item)) {
            all_primitives = false;
            break;
        }
    }
    
    AR__HEAP__FREE(items);
    return all_primitives;
}

/**
 * Get the integer value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 * @note Ownership: Does not take ownership of the data parameter.
 */
int ar_data__get_integer(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__INTEGER) {
        return 0;
    }
    return ref_data->data.int_value;
}

/**
 * Get the double value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 * @note Ownership: Does not take ownership of the data parameter.
 */
double ar_data__get_double(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__DOUBLE) {
        return 0.0;
    }
    return ref_data->data.double_value;
}

/**
 * Get the string value from a data structure
 * @param ref_data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 * @note Ownership: Does not take ownership of the data parameter.
 */
const char *ar_data__get_string(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__STRING) {
        return NULL;
    }
    return ref_data->data.own_string;
}

/**
 * Get the list value from a data structure (read-only)
 * This is a PRIVATE function only for use within the data module.
 * @param ref_data Pointer to the data to retrieve from
 * @return The list value or NULL if data is NULL or not a list type
 * @note Ownership: Does not take ownership of the ref_data parameter.
 *       Returns a borrowed reference that caller must not destroy.
 */
static ar_list_t *ar_data__get_list(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    return ref_data->data.own_list;
}

/**
 * Get the map value from a data structure (read-only)
 * This is a PRIVATE function only for use within the data module.
 * @param ref_data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 * @note Ownership: Does not take ownership of the ref_data parameter.
 *       Returns a borrowed reference that caller must not destroy.
 */
static ar_map_t *ar_data__get_map(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__MAP) {
        return NULL;
    }
    return ref_data->data.own_map;
}

/**
 * Get a data value from a map data structure by key or path
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 * @note Ownership: Does not take ownership of the parameters. Returns a borrowed reference.
 */
ar_data_t *ar_data__get_map_data(const ar_data_t *ref_data, const char *ref_key) {
    if (!ref_data || !ref_key || ref_data->type != AR_DATA_TYPE__MAP) {
        return NULL;
    }
    
    // Get the map from the data
    ar_map_t *ref_map = ar_data__get_map(ref_data);
    if (!ref_map) {
        return NULL;
    }
    
    // Check if the path contains any dots
    if (strchr(ref_key, '.') == NULL) {
        // No dots, do a direct lookup
        return ar_map__get(ref_map, ref_key);
    }
    
    // Create a path object for parsing
    ar_path_t *own_path = ar_path__create_variable(ref_key);
    if (!own_path) {
        return NULL;
    }
    
    // Count path segments for multi-segment paths
    size_t segment_count = ar_path__get_segment_count(own_path);
    
    // Keep track of current data as we traverse the path
    ar_data_t *result = NULL;
    const ar_data_t *ref_current_data = ref_data;
    
    // Process each segment
    for (size_t i = 0; i < segment_count; i++) {
        // Get the current segment
        const char *ref_segment = ar_path__get_segment(own_path, i);
        if (!ref_segment) {
            ar_path__destroy(own_path);
            return NULL;
        }
        
        // Get the map from the current data
        ar_map_t *ref_current_map = ar_data__get_map(ref_current_data);
        if (!ref_current_map) {
            ar_path__destroy(own_path);
            return NULL;
        }
        
        // Get the value for this segment
        result = ar_map__get(ref_current_map, ref_segment);
        
        if (!result) {
            ar_path__destroy(own_path);
            return NULL;
        }
        
        // For all but the last segment, the value must be a map
        if (i < segment_count - 1 && result->type != AR_DATA_TYPE__MAP) {
            ar_path__destroy(own_path);
            return NULL;
        }
        
        ref_current_data = result;
    }
    
    ar_path__destroy(own_path);
    return result;
}

/**
 * Get an integer value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 * @note Ownership: Does not take ownership of the parameters.
 */
int ar_data__get_map_integer(const ar_data_t *ref_data, const char *ref_key) {
    if (!ref_data || !ref_key || ref_data->type != AR_DATA_TYPE__MAP) {
        return 0;
    }
    
    const ar_data_t *ref_value = ar_data__get_map_data(ref_data, ref_key);
    
    if (!ref_value) {
        return 0;
    }
    
    return ar_data__get_integer(ref_value);
}

/**
 * Get a double value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 * @note Ownership: Does not take ownership of the parameters.
 */
double ar_data__get_map_double(const ar_data_t *ref_data, const char *ref_key) {
    if (!ref_data || !ref_key || ref_data->type != AR_DATA_TYPE__MAP) {
        printf("ar_data__get_map_double - invalid parameters: data=%p, key=%s, data_type=%d\n", 
               (const void*)ref_data, ref_key ? ref_key : "NULL", ref_data ? (int)ref_data->type : -1);
        return 0.0;
    }
    
    const ar_data_t *ref_value = ar_data__get_map_data(ref_data, ref_key);
    
    if (!ref_value) {
        printf("ar_data__get_map_double - value not found for key: %s\n", ref_key);
        return 0.0;
    }
    
    printf("ar_data__get_map_double - found value of type: %d for key: %s\n", 
           (int)ref_value->type, ref_key);
    
    double result = ar_data__get_double(ref_value);
    printf("ar_data__get_map_double - result: %f\n", result);
    
    return result;
}

/**
 * Get a string value from a map data structure by key
 * @param ref_data Pointer to the map data to retrieve from
 * @param ref_key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 * @note Ownership: Does not take ownership of the parameters.
 */
const char *ar_data__get_map_string(const ar_data_t *ref_data, const char *ref_key) {
    if (!ref_data || !ref_key || ref_data->type != AR_DATA_TYPE__MAP) {
        return NULL;
    }
    
    const ar_data_t *ref_value = ar_data__get_map_data(ref_data, ref_key);
    
    if (!ref_value) {
        return NULL;
    }
    
    return ar_data__get_string(ref_value);
}


/**
 * NOTE: The set_map_* functions use ar_data__get_map_data to verify that the path exists 
 * and fail if it doesn't. These functions are implemented recursively, traversing the 
 * path to the final key and using the base case implementation for the leaf node.
 * Clients must explicitly create intermediate maps before setting values in nested paths.
 * 
 * All set_map_* functions use ar_data__set_map_data for the actual implementation to reduce code duplication.
 */

/**
 * Set an integer value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_integer(ar_data_t *mut_data, const char *ref_key, int value) {
    if (!mut_data || !ref_key || mut_data->type != AR_DATA_TYPE__MAP) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_int_data = ar_data__create_integer(value);
    if (!own_int_data) {
        return false;
    }
    
    // Use the common set_map_data function
    bool result = ar_data__set_map_data(mut_data, ref_key, own_int_data);
    
    // If set_map_data failed, we need to free own_int_data manually
    if (!result) {
        ar_data__destroy(own_int_data);
    }
    // If successful, ownership is transferred to the map
    
    return result;
}

/**
 * Set a double value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_double(ar_data_t *mut_data, const char *ref_key, double value) {
    if (!mut_data || !ref_key || mut_data->type != AR_DATA_TYPE__MAP) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_double_data = ar_data__create_double(value);
    if (!own_double_data) {
        return false;
    }
    
    // Use the common set_map_data function
    bool result = ar_data__set_map_data(mut_data, ref_key, own_double_data);
    
    // If set_map_data failed, we need to free own_double_data manually
    if (!result) {
        ar_data__destroy(own_double_data);
    }
    // If successful, ownership is transferred to the map
    
    return result;
}

/**
 * Set a string value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
/**
 * Set a data value in a map data structure by key or path
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param own_value The data value to store (ownership is transferred)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 */
bool ar_data__set_map_data(ar_data_t *mut_data, const char *ref_key, ar_data_t *own_value) {
    if (!mut_data || !ref_key || mut_data->type != AR_DATA_TYPE__MAP || !own_value) {
        return false;
    }
    
    // For simple keys with no dots, use direct access
    if (strchr(ref_key, '.') == NULL) {
        ar_map_t *map = mut_data->data.own_map;
        if (!map) {
            return false;
        }
        
        // Get the existing data for later cleanup
        ar_data_t *prev_data = ar_map__get(map, ref_key);
        
        // Create a copy of the key
        char *key_copy = AR__HEAP__STRDUP(ref_key, "Map key copy");
        if (!key_copy) {
            return false;
        }
        
        // Map should hold ownership of the data
        if (!ar_data__hold_ownership(own_value, mut_data)) {
            // Data is already owned by someone else
            AR__HEAP__FREE(key_copy);
            return false;
        }
        
        // Set the new value
        if (!ar_map__set(map, key_copy, own_value)) {
            // Failed to set, release ownership
            ar_data__transfer_ownership(own_value, mut_data);
            AR__HEAP__FREE(key_copy);
            return false;
        }
        
        // Add the key to our tracking list
        if (!ar_list__add_last(mut_data->own_keys, key_copy)) {
            // Unlikely, but handle failure
            ar_map__set(map, ref_key, prev_data); // Try to restore previous state
            ar_data__transfer_ownership(own_value, mut_data);
            AR__HEAP__FREE(key_copy);
            return false;
        }
        
        // Free the old data after successful update
        if (prev_data) {
            // Transfer ownership back from map to NULL before destroying
            ar_data__transfer_ownership(prev_data, mut_data);
            ar_data__destroy(prev_data);
        }
        
        return true;
    }
    
    // Handle path-based access for keys with dots
    // Create a path object for parsing
    ar_path_t *own_path = ar_path__create_variable(ref_key);
    if (!own_path) {
        return false;
    }
    
    size_t segment_count = ar_path__get_segment_count(own_path);
    if (segment_count == 0) {
        ar_path__destroy(own_path);
        return false;
    }
    
    // Get the parent path
    ar_path_t *own_parent_path = ar_path__get_parent(own_path);
    if (!own_parent_path) {
        ar_path__destroy(own_path);
        return false;
    }
    
    // Get the final key segment
    const char *ref_final_key = ar_path__get_segment(own_path, segment_count - 1);
    if (!ref_final_key) {
        ar_path__destroy(own_parent_path);
        ar_path__destroy(own_path);
        return false;
    }
    
    // Get the parent map data - this will fail if any part of the path doesn't exist
    // or if any part of the path is not a map
    const char *ref_parent_path_str = ar_path__get_string(own_parent_path);
    ar_data_t *parent_data = ar_data__get_map_data(mut_data, ref_parent_path_str);
    if (!parent_data || ar_data__get_type(parent_data) != AR_DATA_TYPE__MAP) {
        ar_path__destroy(own_parent_path);
        ar_path__destroy(own_path);
        return false;
    }
    
    // Recursively call set_map_data with the parent data and final key
    bool success = ar_data__set_map_data(parent_data, ref_final_key, own_value);
    
    // The recursive call now completely handles the ownership of own_value
    // If successful, ownership was transferred to parent_data
    // If not successful, the recursive call will have freed own_value
    
    ar_path__destroy(own_parent_path);
    ar_path__destroy(own_path);
    return success;
}

/**
 * Set a string value in a map data structure by key
 * @param mut_data Pointer to the map data to modify
 * @param ref_key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param ref_value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__set_map_string(ar_data_t *mut_data, const char *ref_key, const char *ref_value) {
    if (!mut_data || !ref_key || mut_data->type != AR_DATA_TYPE__MAP) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_string_data = ar_data__create_string(ref_value);
    if (!own_string_data) {
        return false;
    }
    
    // Use the common set_map_data function
    bool result = ar_data__set_map_data(mut_data, ref_key, own_string_data);
    
    // If set_map_data failed, we need to free own_string_data manually
    if (!result) {
        ar_data__destroy(own_string_data);
    }
    // If successful, ownership is transferred to the map
    
    return result;
}

/**
 * Add an integer value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_integer(ar_data_t *mut_data, int value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_int_data = ar_data__create_integer(value);
    if (!own_int_data) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        ar_data__destroy(own_int_data);
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list__add_first(ref_list, own_int_data)) {
        ar_data__destroy(own_int_data);
        return false;
    }
    
    return true;
}

/**
 * Add a double value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_double(ar_data_t *mut_data, double value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_double_data = ar_data__create_double(value);
    if (!own_double_data) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        ar_data__destroy(own_double_data);
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list__add_first(ref_list, own_double_data)) {
        ar_data__destroy(own_double_data);
        return false;
    }
    
    return true;
}

/**
 * Add a string value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_first_string(ar_data_t *mut_data, const char *ref_value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_string_data = ar_data__create_string(ref_value);
    if (!own_string_data) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        ar_data__destroy(own_string_data);
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list__add_first(ref_list, own_string_data)) {
        ar_data__destroy(own_string_data);
        return false;
    }
    
    return true;
}

/**
 * Add a data value to the beginning of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 *       While ar_list__add_first does not take ownership directly,
 *       this function takes ownership as part of the API contract.
 */
bool ar_data__list_add_first_data(ar_data_t *mut_data, ar_data_t *own_value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST || !own_value) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        return false;
    }
    
    // List should hold ownership of the data
    if (!ar_data__hold_ownership(own_value, mut_data)) {
        // Data is already owned by someone else
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list__add_first(ref_list, own_value)) {
        // Failed to add, release ownership
        ar_data__transfer_ownership(own_value, mut_data);
        return false;
    }
    
    // Note: We don't set own_value = NULL here because ownership is handled internally.
    // The caller will set own_value = NULL after calling this function, per the API contract.
    
    return true;
}

/**
 * Add an integer value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_integer(ar_data_t *mut_data, int value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_int_data = ar_data__create_integer(value);
    if (!own_int_data) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        ar_data__destroy(own_int_data);
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list__add_last(ref_list, own_int_data)) {
        ar_data__destroy(own_int_data);
        return false;
    }
    
    return true;
}

/**
 * Add a double value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_double(ar_data_t *mut_data, double value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_double_data = ar_data__create_double(value);
    if (!own_double_data) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        ar_data__destroy(own_double_data);
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list__add_last(ref_list, own_double_data)) {
        ar_data__destroy(own_double_data);
        return false;
    }
    
    return true;
}

/**
 * Add a string value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param ref_value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Does not take ownership of the parameters.
 */
bool ar_data__list_add_last_string(ar_data_t *mut_data, const char *ref_value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return false;
    }
    
    // Create new data
    ar_data_t *own_string_data = ar_data__create_string(ref_value);
    if (!own_string_data) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        ar_data__destroy(own_string_data);
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list__add_last(ref_list, own_string_data)) {
        ar_data__destroy(own_string_data);
        return false;
    }
    
    return true;
}

/**
 * Add a data value to the end of a list data structure
 * @param mut_data Pointer to the list data to modify
 * @param own_value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 * @note Ownership: Takes ownership of the own_value parameter.
 *       The caller should set own_value = NULL after this call.
 *       While ar_list__add_last does not take ownership directly,
 *       this function takes ownership as part of the API contract.
 */
bool ar_data__list_add_last_data(ar_data_t *mut_data, ar_data_t *own_value) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST || !own_value) {
        return false;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        return false;
    }
    
    // List should hold ownership of the data
    if (!ar_data__hold_ownership(own_value, mut_data)) {
        // Data is already owned by someone else
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list__add_last(ref_list, own_value)) {
        // Failed to add, release ownership
        ar_data__transfer_ownership(own_value, mut_data);
        return false;
    }
    
    // Note: We don't set own_value = NULL here because ownership is handled internally.
    // The caller will set own_value = NULL after calling this function, per the API contract.
    
    return true;
}

/**
 * Helper to transfer ownership when removing data from a collection
 * @param removed The data that was removed
 * @param collection The collection that owned the data
 */
static void _transfer_ownership_on_remove(ar_data_t *removed, ar_data_t *collection) {
    if (removed && removed->owner == collection) {
        // Transfer ownership from collection to NULL (making it unowned)
        ar_data__transfer_ownership(removed, collection);
    }
}

/**
 * Remove and return the first data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t *ar_data__list_remove_first(ar_data_t *mut_data) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        return NULL;
    }
    
    // Remove the first item from the list
    ar_data_t *removed = (ar_data_t *)ar_list__remove_first(ref_list);
    _transfer_ownership_on_remove(removed, mut_data);
    return removed; // Ownership transferred to caller
}

/**
 * Remove and return the last data value from a list data structure
 * @param mut_data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t *ar_data__list_remove_last(ar_data_t *mut_data) {
    if (!mut_data || mut_data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(mut_data);
    if (!ref_list) {
        return NULL;
    }
    
    // Remove the last item from the list
    ar_data_t *removed = (ar_data_t *)ar_list__remove_last(ref_list);
    _transfer_ownership_on_remove(removed, mut_data);
    return removed; // Ownership transferred to caller
}

/**
 * Get the first data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The first data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
ar_data_t *ar_data__list_first(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(ref_data);
    if (!ref_list) {
        return NULL;
    }
    
    // Get the first item from the list
    return (ar_data_t *)ar_list__first(ref_list);
}

/**
 * Get the last data value from a list data structure (without removing it)
 * @param ref_data Pointer to the list data
 * @return The last data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
ar_data_t *ar_data__list_last(const ar_data_t *ref_data) {
    if (!ref_data || ref_data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    
    // Get the list from the data
    ar_list_t *ref_list = ar_data__get_list(ref_data);
    if (!ref_list) {
        return NULL;
    }
    
    // Get the last item from the list
    return (ar_data_t *)ar_list__last(ref_list);
}

/**
 * Remove and return the first integer value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or first item not an integer
 * @note This function also removes and frees the data structure containing the integer
 */
int ar_data__list_remove_first_integer(ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return 0;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return 0;
    }
    
    // Remove the first item from the list
    ar_data_t *first_data = (ar_data_t *)ar_list__remove_first(list);
    if (!first_data) {
        return 0;
    }
    
    // Check if the data is an integer
    if (ar_data__get_type(first_data) != AR_DATA_TYPE__INTEGER) {
        // Put the item back in the list if it's not an integer
        ar_list__add_first(list, first_data);
        return 0;
    }
    
    // Get the integer value
    int value = ar_data__get_integer(first_data);
    
    // Free the data structure
    ar_data__destroy(first_data);
    
    return value;
}

/**
 * Remove and return the first double value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or first item not a double
 * @note This function also removes and frees the data structure containing the double
 */
double ar_data__list_remove_first_double(ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return 0.0;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return 0.0;
    }
    
    // Remove the first item from the list
    ar_data_t *first_data = (ar_data_t *)ar_list__remove_first(list);
    if (!first_data) {
        return 0.0;
    }
    
    // Check if the data is a double
    if (ar_data__get_type(first_data) != AR_DATA_TYPE__DOUBLE) {
        // Put the item back in the list if it's not a double
        ar_list__add_first(list, first_data);
        return 0.0;
    }
    
    // Get the double value
    double value = ar_data__get_double(first_data);
    
    // Free the data structure
    ar_data__destroy(first_data);
    
    return value;
}

/**
 * Remove and return the first string value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or first item not a string
 * @note This function also removes and frees the data structure containing the string reference
 */
char *ar_data__list_remove_first_string(ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return NULL;
    }
    
    // Remove the first item from the list
    ar_data_t *first_data = (ar_data_t *)ar_list__remove_first(list);
    if (!first_data) {
        return NULL;
    }
    
    // Check if the data is a string
    if (ar_data__get_type(first_data) != AR_DATA_TYPE__STRING) {
        // Put the item back in the list if it's not a string
        ar_list__add_first(list, first_data);
        return NULL;
    }
    
    // Get the string value
    const char *orig_str = ar_data__get_string(first_data);
    if (!orig_str) {
        ar_data__destroy(first_data);
        return NULL;
    }
    
    // Make a copy of the string for the caller
    char *str_copy = AR__HEAP__STRDUP(orig_str, "String copy for conversion");
    
    // Free the data structure
    ar_data__destroy(first_data);
    
    return str_copy;
}

/**
 * Remove and return the last integer value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The integer value, or 0 if data is NULL, not a list, list is empty, or last item not an integer
 * @note This function also removes and frees the data structure containing the integer
 */
int ar_data__list_remove_last_integer(ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return 0;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return 0;
    }
    
    // Remove the last item from the list
    ar_data_t *last_data = (ar_data_t *)ar_list__remove_last(list);
    if (!last_data) {
        return 0;
    }
    
    // Check if the data is an integer
    if (ar_data__get_type(last_data) != AR_DATA_TYPE__INTEGER) {
        // Put the item back in the list if it's not an integer
        ar_list__add_last(list, last_data);
        return 0;
    }
    
    // Get the integer value
    int value = ar_data__get_integer(last_data);
    
    // Free the data structure
    ar_data__destroy(last_data);
    
    return value;
}

/**
 * Remove and return the last double value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The double value, or 0.0 if data is NULL, not a list, list is empty, or last item not a double
 * @note This function also removes and frees the data structure containing the double
 */
double ar_data__list_remove_last_double(ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return 0.0;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return 0.0;
    }
    
    // Remove the last item from the list
    ar_data_t *last_data = (ar_data_t *)ar_list__remove_last(list);
    if (!last_data) {
        return 0.0;
    }
    
    // Check if the data is a double
    if (ar_data__get_type(last_data) != AR_DATA_TYPE__DOUBLE) {
        // Put the item back in the list if it's not a double
        ar_list__add_last(list, last_data);
        return 0.0;
    }
    
    // Get the double value
    double value = ar_data__get_double(last_data);
    
    // Free the data structure
    ar_data__destroy(last_data);
    
    return value;
}

/**
 * Remove and return the last string value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The string value (caller must free), or NULL if data is NULL, not a list, list is empty, or last item not a string
 * @note This function also removes and frees the data structure containing the string reference
 */
char *ar_data__list_remove_last_string(ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return NULL;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return NULL;
    }
    
    // Remove the last item from the list
    ar_data_t *last_data = (ar_data_t *)ar_list__remove_last(list);
    if (!last_data) {
        return NULL;
    }
    
    // Check if the data is a string
    if (ar_data__get_type(last_data) != AR_DATA_TYPE__STRING) {
        // Put the item back in the list if it's not a string
        ar_list__add_last(list, last_data);
        return NULL;
    }
    
    // Get the string value
    const char *orig_str = ar_data__get_string(last_data);
    if (!orig_str) {
        ar_data__destroy(last_data);
        return NULL;
    }
    
    // Make a copy of the string for the caller
    char *str_copy = AR__HEAP__STRDUP(orig_str, "String copy for conversion");
    
    // Free the data structure
    ar_data__destroy(last_data);
    
    return str_copy;
}

/**
 * Get the number of items in a list data structure
 * @param data Pointer to the list data
 * @return The number of items, or 0 if data is NULL or not a list
 */
size_t ar_data__list_count(const ar_data_t *data) {
    if (!data || data->type != AR_DATA_TYPE__LIST) {
        return 0;
    }
    
    // Get the list from the data
    ar_list_t *list = ar_data__get_list(data);
    if (!list) {
        return 0;
    }
    
    // Get the count from the list
    return ar_list__count(list);
}

/**
 * Get all keys from a map data structure
 * @param ref_data Pointer to the map data to retrieve keys from
 * @return A list containing string data values for each key, or NULL if data is NULL or not a map
 * @note Ownership: Returns an owned list that caller must destroy.
 *       The returned list contains string data values (not raw strings).
 *       An empty map returns an empty list (not NULL).
 */
ar_data_t* ar_data__get_map_keys(const ar_data_t *ref_data) {
    // Check if input is NULL
    if (!ref_data) {
        return NULL;
    }
    
    // Check if input is a map type
    if (ref_data->type != AR_DATA_TYPE__MAP) {
        return NULL;
    }
    
    // Create a new list to hold the keys
    ar_data_t *own_keys_list = ar_data__create_list();
    if (!own_keys_list) {
        return NULL;
    }
    
    // Get the internal keys list
    if (!ref_data->own_keys) {
        // Return empty list for maps with no keys
        return own_keys_list;
    }
    
    // Get all keys from the internal list
    void **own_key_ptrs = ar_list__items(ref_data->own_keys);
    size_t key_count = ar_list__count(ref_data->own_keys);
    
    // Add each key to the result list as a string data value
    for (size_t i = 0; i < key_count; i++) {
        const char *ref_key = (const char *)own_key_ptrs[i];
        
        // Create a string data value for the key
        ar_data_t *own_key_data = ar_data__create_string(ref_key);
        if (!own_key_data) {
            // Clean up on failure
            AR__HEAP__FREE(own_key_ptrs);
            ar_data__destroy(own_keys_list);
            return NULL;
        }
        
        // Add the key data to the list
        if (!ar_data__list_add_last_data(own_keys_list, own_key_data)) {
            // Clean up on failure
            ar_data__destroy(own_key_data);
            AR__HEAP__FREE(own_key_ptrs);
            ar_data__destroy(own_keys_list);
            return NULL;
        }
    }
    
    // Free the key array (we own it)
    AR__HEAP__FREE(own_key_ptrs);
    
    return own_keys_list;
}
