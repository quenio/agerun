#include "agerun_data.h"
#include "agerun_string.h"
#include "agerun_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Data structure for storing various data types
 */
struct data_s {
    data_type_t type;
    union {
        int int_value;
        double double_value;
        char *string_ref;
        list_t *list_ref;
        map_t *map_ref;
    } data;
    list_t *keys;  // List of keys that belong to this data's map (only used for DATA_MAP type)
};

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_integer(int value) {
    data_t* data = (data_t*)malloc(sizeof(data_t));
    if (!data) {
        return NULL;
    }
    
    data->type = DATA_INTEGER;
    data->data.int_value = value;
    data->keys = NULL;
    return data;
}

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_double(double value) {
    data_t* data = (data_t*)malloc(sizeof(data_t));
    if (!data) {
        return NULL;
    }
    
    data->type = DATA_DOUBLE;
    data->data.double_value = value;
    data->keys = NULL;
    return data;
}

/**
 * Create a new string data value
 * @param value String value to initialize with (will be copied)
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_string(const char *value) {
    data_t* data = (data_t*)malloc(sizeof(data_t));
    if (!data) {
        return NULL;
    }
    
    data->type = DATA_STRING;
    data->data.string_ref = value ? strdup(value) : NULL;
    if (value && !data->data.string_ref) {
        free(data);
        return NULL;
    }
    
    data->keys = NULL;
    return data;
}

/**
 * Create a new list data value
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_list(void) {
    data_t* data = (data_t*)malloc(sizeof(data_t));
    if (!data) {
        return NULL;
    }
    
    data->type = DATA_LIST;
    data->data.list_ref = ar_list_create();
    if (!data->data.list_ref) {
        free(data);
        return NULL;
    }
    
    data->keys = NULL;
    return data;
}

/**
 * Create a new map data value
 * @return Pointer to the new data, or NULL on failure
 */
data_t* ar_data_create_map(void) {
    data_t* data = (data_t*)malloc(sizeof(data_t));
    if (!data) {
        return NULL;
    }
    
    data->type = DATA_MAP;
    data->data.map_ref = ar_map_create();
    if (!data->data.map_ref) {
        free(data);
        return NULL;
    }
    
    // Create a list to track keys
    data->keys = ar_list_create();
    if (!data->keys) {
        ar_map_destroy(data->data.map_ref);
        free(data);
        return NULL;
    }
    
    return data;
}

// This function has been replaced by ar_map_refs

/**
 * Free resources associated with a data structure and release memory
 * @param data Pointer to the data to destroy
 */
void ar_data_destroy(data_t *data) {
    if (!data) return;
    
    if (data->type == DATA_STRING && data->data.string_ref) {
        free(data->data.string_ref);
        data->data.string_ref = NULL;
    } else if (data->type == DATA_LIST && data->data.list_ref) {
        // For lists, we need to:
        // 1. Get all data values stored in the list (for later cleanup)
        // 2. Free the list structure itself
        // 3. Then free all the data values
        
        // Get all data values for later cleanup
        void **items = ar_list_items(data->data.list_ref);
        size_t item_count = ar_list_count(data->data.list_ref);
        
        // Destroy the list structure first
        ar_list_destroy(data->data.list_ref);
        data->data.list_ref = NULL;
        
        // Free all data values
        if (items) {
            for (size_t i = 0; i < item_count; i++) {
                ar_data_destroy((data_t*)items[i]);
            }
            free(items); // Free the array itself
        }
    } else if (data->type == DATA_MAP && data->data.map_ref) {
        // For maps, we need to:
        // 1. Get all data values stored in the map (for later cleanup)
        // 2. Free the map structure itself
        // 3. Free all keys tracked in our list
        // 4. Free the list itself
        // 5. Then free all the data values
        
        // Get all data values for later cleanup
        void **refs = ar_map_refs(data->data.map_ref);
        size_t ref_count = ar_map_count(data->data.map_ref);
        
        // Destroy the map structure first
        ar_map_destroy(data->data.map_ref);
        data->data.map_ref = NULL;
        
        // Free all tracked keys
        if (data->keys) {
            void **key_ptrs = ar_list_items(data->keys);
            size_t key_count = ar_list_count(data->keys);
            
            if (key_ptrs) {
                for (size_t i = 0; i < key_count; i++) {
                    free(key_ptrs[i]); // Free each key string
                }
                free(key_ptrs); // Free the array itself
            }
            
            // Destroy the key tracking list
            ar_list_destroy(data->keys);
            data->keys = NULL;
        }
        
        // Free all data values
        if (refs) {
            for (size_t i = 0; i < ref_count; i++) {
                ar_data_destroy((data_t*)refs[i]);
            }
            free(refs); // Free the array itself
        }
    }
    
    free(data);
}

/**
 * Get the type of a data structure
 * @param data Pointer to the data to check
 * @return The data type or DATA_INTEGER if data is NULL
 */
data_type_t ar_data_get_type(const data_t *data) {
    if (!data) {
        return DATA_INTEGER; // Default to int if NULL
    }
    return data->type;
}

/**
 * Get the integer value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The integer value or 0 if data is NULL or not an integer type
 */
int ar_data_get_integer(const data_t *data) {
    if (!data || data->type != DATA_INTEGER) {
        return 0;
    }
    return data->data.int_value;
}

/**
 * Get the double value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The double value or 0.0 if data is NULL or not a double type
 */
double ar_data_get_double(const data_t *data) {
    if (!data || data->type != DATA_DOUBLE) {
        return 0.0;
    }
    return data->data.double_value;
}

/**
 * Get the string value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The string value or NULL if data is NULL or not a string type
 */
const char *ar_data_get_string(const data_t *data) {
    if (!data || data->type != DATA_STRING) {
        return NULL;
    }
    return data->data.string_ref;
}

/**
 * Get the list value from a data structure (read-only)
 * This is a PRIVATE function only for use within the data module.
 * @param data Pointer to the data to retrieve from
 * @return The list value or NULL if data is NULL or not a list type
 */
static list_t *ar_data_get_list(const data_t *data) {
    if (!data || data->type != DATA_LIST) {
        return NULL;
    }
    return data->data.list_ref;
}

/**
 * Get the map value from a data structure (read-only)
 * This is a PRIVATE function only for use within the data module.
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
static map_t *ar_data_get_map(const data_t *data) {
    if (!data || data->type != DATA_MAP) {
        return NULL;
    }
    return data->data.map_ref;
}

/**
 * Get a data value from a map data structure by key or path
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 */
data_t *ar_data_get_map_data(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return NULL;
    }
    
    // Get the map from the data
    map_t *map = ar_data_get_map(data);
    if (!map) {
        return NULL;
    }
    
    // Check if the path contains any dots
    if (strchr(key, '.') == NULL) {
        // No dots, do a direct lookup
        return ar_map_get(map, key);
    }
    
    // Count path segments for multi-segment paths
    size_t segment_count = ar_string_path_count(key, '.');
    
    // Keep track of current data as we traverse the path
    data_t *result = NULL;
    const data_t *current_data = data;
    
    // Process each segment
    for (size_t i = 0; i < segment_count; i++) {
        // Get the current segment
        char *segment = ar_string_path_segment(key, '.', i);
        if (!segment) {
            return NULL;
        }
        
        // Get the map from the current data
        map_t *current_map = ar_data_get_map(current_data);
        if (!current_map) {
            free(segment);
            return NULL;
        }
        
        // Get the value for this segment
        result = ar_map_get(current_map, segment);
        
        if (!result) {
            free(segment);
            return NULL;
        }
        
        // For all but the last segment, the value must be a map
        if (i < segment_count - 1 && result->type != DATA_MAP) {
            free(segment);
            return NULL;
        }
        
        current_data = result;
        free(segment);
    }
    
    return result;
}

/**
 * Get an integer value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 */
int ar_data_get_map_integer(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return 0;
    }
    
    const data_t *value = ar_data_get_map_data(data, key);
    
    if (!value) {
        return 0;
    }
    
    return ar_data_get_integer(value);
}

/**
 * Get a double value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 */
double ar_data_get_map_double(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        printf("ar_data_get_map_double - invalid parameters: data=%p, key=%s, data_type=%d\n", 
               (const void*)data, key ? key : "NULL", data ? (int)data->type : -1);
        return 0.0;
    }
    
    const data_t *value = ar_data_get_map_data(data, key);
    
    if (!value) {
        printf("ar_data_get_map_double - value not found for key: %s\n", key);
        return 0.0;
    }
    
    printf("ar_data_get_map_double - found value of type: %d for key: %s\n", 
           (int)value->type, key);
    
    double result = ar_data_get_double(value);
    printf("ar_data_get_map_double - result: %f\n", result);
    
    return result;
}

/**
 * Get a string value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 */
const char *ar_data_get_map_string(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return NULL;
    }
    
    const data_t *value = ar_data_get_map_data(data, key);
    
    if (!value) {
        return NULL;
    }
    
    return ar_data_get_string(value);
}


/**
 * NOTE: The set_map_* functions use ar_data_get_map_data to verify that the path exists 
 * and fail if it doesn't. These functions are implemented recursively, traversing the 
 * path to the final key and using the base case implementation for the leaf node.
 * Clients must explicitly create intermediate maps before setting values in nested paths.
 * 
 * All set_map_* functions use ar_data_set_map_data for the actual implementation to reduce code duplication.
 */

/**
 * Set an integer value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The integer value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_integer(data_t *data, const char *key, int value) {
    if (!data || !key || data->type != DATA_MAP) {
        return false;
    }
    
    // Create new data
    data_t *int_data = ar_data_create_integer(value);
    if (!int_data) {
        return false;
    }
    
    // Use the common set_map_data function
    return ar_data_set_map_data(data, key, int_data);
}

/**
 * Set a double value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The double value to store
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_double(data_t *data, const char *key, double value) {
    if (!data || !key || data->type != DATA_MAP) {
        return false;
    }
    
    // Create new data
    data_t *double_data = ar_data_create_double(value);
    if (!double_data) {
        return false;
    }
    
    // Use the common set_map_data function
    return ar_data_set_map_data(data, key, double_data);
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
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The data value to store (ownership is transferred)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_data(data_t *data, const char *key, data_t *value) {
    if (!data || !key || data->type != DATA_MAP || !value) {
        return false;
    }
    
    // For simple keys with no dots, use direct access
    if (strchr(key, '.') == NULL) {
        map_t *map = data->data.map_ref;
        if (!map) {
            return false;
        }
        
        // Get the existing data for later cleanup
        data_t *prev_data = ar_map_get(map, key);
        
        // Create a copy of the key
        char *key_copy = strdup(key);
        if (!key_copy) {
            return false;
        }
        
        // Set the new value
        if (!ar_map_set(map, key_copy, value)) {
            free(key_copy);
            return false;
        }
        
        // Add the key to our tracking list
        if (!ar_list_add_last(data->keys, key_copy)) {
            // Unlikely, but handle failure
            ar_map_set(map, key, prev_data); // Try to restore previous state
            free(key_copy);
            return false;
        }
        
        // Free the old data after successful update
        if (prev_data) {
            ar_data_destroy(prev_data);
        }
        
        return true;
    }
    
    // Handle path-based access for keys with dots
    // Extract the parent path and final key
    size_t segment_count = ar_string_path_count(key, '.');
    if (segment_count == 0) {
        return false;
    }
    
    // Get the parent path using ar_string_path_parent
    char *parent_path = ar_string_path_parent(key, '.');
    if (!parent_path) {
        return false;
    }
    
    // Get the final key segment
    char *final_key = ar_string_path_segment(key, '.', segment_count - 1);
    if (!final_key) {
        free(parent_path);
        return false;
    }
    
    // Get the parent map data - this will fail if any part of the path doesn't exist
    // or if any part of the path is not a map
    data_t *parent_data = ar_data_get_map_data(data, parent_path);
    if (!parent_data || ar_data_get_type(parent_data) != DATA_MAP) {
        free(final_key);
        free(parent_path);
        return false;
    }
    
    // Recursively call set_map_data with the parent data and final key
    bool success = ar_data_set_map_data(parent_data, final_key, value);
    
    // If not successful, we need to destroy the value since ownership wasn't transferred
    if (!success) {
        ar_data_destroy(value);
    }
    
    free(final_key);
    free(parent_path);
    return success;
}

bool ar_data_set_map_string(data_t *data, const char *key, const char *value) {
    if (!data || !key || data->type != DATA_MAP) {
        return false;
    }
    
    // Create new data
    data_t *string_data = ar_data_create_string(value);
    if (!string_data) {
        return false;
    }
    
    // Use the common set_map_data function
    return ar_data_set_map_data(data, key, string_data);
}

/**
 * Add an integer value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_integer(data_t *data, int value) {
    if (!data || data->type != DATA_LIST) {
        return false;
    }
    
    // Create new data
    data_t *int_data = ar_data_create_integer(value);
    if (!int_data) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        ar_data_destroy(int_data);
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list_add_first(list, int_data)) {
        ar_data_destroy(int_data);
        return false;
    }
    
    return true;
}

/**
 * Add a double value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_double(data_t *data, double value) {
    if (!data || data->type != DATA_LIST) {
        return false;
    }
    
    // Create new data
    data_t *double_data = ar_data_create_double(value);
    if (!double_data) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        ar_data_destroy(double_data);
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list_add_first(list, double_data)) {
        ar_data_destroy(double_data);
        return false;
    }
    
    return true;
}

/**
 * Add a string value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_string(data_t *data, const char *value) {
    if (!data || data->type != DATA_LIST) {
        return false;
    }
    
    // Create new data
    data_t *string_data = ar_data_create_string(value);
    if (!string_data) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        ar_data_destroy(string_data);
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list_add_first(list, string_data)) {
        ar_data_destroy(string_data);
        return false;
    }
    
    return true;
}

/**
 * Add a data value to the beginning of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_first_data(data_t *data, data_t *value) {
    if (!data || data->type != DATA_LIST || !value) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return false;
    }
    
    // Add the data to the beginning of the list
    if (!ar_list_add_first(list, value)) {
        return false;
    }
    
    return true;
}

/**
 * Add an integer value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The integer value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_integer(data_t *data, int value) {
    if (!data || data->type != DATA_LIST) {
        return false;
    }
    
    // Create new data
    data_t *int_data = ar_data_create_integer(value);
    if (!int_data) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        ar_data_destroy(int_data);
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list_add_last(list, int_data)) {
        ar_data_destroy(int_data);
        return false;
    }
    
    return true;
}

/**
 * Add a double value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The double value to add
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_double(data_t *data, double value) {
    if (!data || data->type != DATA_LIST) {
        return false;
    }
    
    // Create new data
    data_t *double_data = ar_data_create_double(value);
    if (!double_data) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        ar_data_destroy(double_data);
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list_add_last(list, double_data)) {
        ar_data_destroy(double_data);
        return false;
    }
    
    return true;
}

/**
 * Add a string value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The string value to add (will be copied)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_string(data_t *data, const char *value) {
    if (!data || data->type != DATA_LIST) {
        return false;
    }
    
    // Create new data
    data_t *string_data = ar_data_create_string(value);
    if (!string_data) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        ar_data_destroy(string_data);
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list_add_last(list, string_data)) {
        ar_data_destroy(string_data);
        return false;
    }
    
    return true;
}

/**
 * Add a data value to the end of a list data structure
 * @param data Pointer to the list data to modify
 * @param value The data value to add (ownership is transferred)
 * @return true if successful, false if data is NULL, not a list, or allocation failure
 */
bool ar_data_list_add_last_data(data_t *data, data_t *value) {
    if (!data || data->type != DATA_LIST || !value) {
        return false;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return false;
    }
    
    // Add the data to the end of the list
    if (!ar_list_add_last(list, value)) {
        return false;
    }
    
    return true;
}

/**
 * Remove and return the first data value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_remove_first(data_t *data) {
    if (!data || data->type != DATA_LIST) {
        return NULL;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return NULL;
    }
    
    // Remove the first item from the list
    return (data_t *)ar_list_remove_first(list);
}

/**
 * Remove and return the last data value from a list data structure
 * @param data Pointer to the list data to modify
 * @return The removed data value (ownership is transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_remove_last(data_t *data) {
    if (!data || data->type != DATA_LIST) {
        return NULL;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return NULL;
    }
    
    // Remove the last item from the list
    return (data_t *)ar_list_remove_last(list);
}

/**
 * Get the first data value from a list data structure (without removing it)
 * @param data Pointer to the list data
 * @return The first data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_first(const data_t *data) {
    if (!data || data->type != DATA_LIST) {
        return NULL;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return NULL;
    }
    
    // Get the first item from the list
    return (data_t *)ar_list_first(list);
}

/**
 * Get the last data value from a list data structure (without removing it)
 * @param data Pointer to the list data
 * @return The last data value (ownership is not transferred), or NULL if data is NULL, not a list, or list is empty
 */
data_t *ar_data_list_last(const data_t *data) {
    if (!data || data->type != DATA_LIST) {
        return NULL;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return NULL;
    }
    
    // Get the last item from the list
    return (data_t *)ar_list_last(list);
}

/**
 * Get the number of items in a list data structure
 * @param data Pointer to the list data
 * @return The number of items, or 0 if data is NULL or not a list
 */
size_t ar_data_list_count(const data_t *data) {
    if (!data || data->type != DATA_LIST) {
        return 0;
    }
    
    // Get the list from the data
    list_t *list = ar_data_get_list(data);
    if (!list) {
        return 0;
    }
    
    // Get the count from the list
    return ar_list_count(list);
}
