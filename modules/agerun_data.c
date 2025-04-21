#include "agerun_data.h"
#include "agerun_string.h"
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
        map_t *map_ref;
    } data;
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
    
    return data;
}

/**
 * Free resources associated with a data structure and release memory
 * @param data Pointer to the data to destroy
 */
void ar_data_destroy(data_t *data) {
    if (!data) return;
    
    if (data->type == DATA_STRING && data->data.string_ref) {
        free(data->data.string_ref);
        data->data.string_ref = NULL;
    } else if (data->type == DATA_MAP && data->data.map_ref) {
        ar_map_destroy(data->data.map_ref);
        data->data.map_ref = NULL;
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
 * Get the map value from a data structure (read-only)
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
map_t *ar_data_get_map(const data_t *data) {
    if (!data || data->type != DATA_MAP) {
        return NULL;
    }
    return data->data.map_ref;
}

/**
 * NOTE: Removed ar_data_get_map_mutable as it's now redundant with ar_data_get_map
 * Both ar_data_get_map and ar_data_get_map_mutable return a mutable map pointer
 */

/**
 * Get a data value from a map data structure by key or path
 * @param data Pointer to the map data to retrieve from
 * @param key The key or path to look up in the map (supports "key.sub_key.sub_sub_key" format)
 * @return The data value, or NULL if data is NULL, not a map, or key not found
 */
const data_t *ar_data_get_map_data(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return NULL;
    }
    
    // Get the map from the data
    const map_t *map = ar_data_get_map(data);
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
    
    // Start with the current data
    const data_t *current_data = data;
    
    // Process each segment
    for (size_t i = 0; i < segment_count; i++) {
        // Get the current segment
        char *segment = ar_string_path_segment(key, '.', i);
        if (!segment) {
            return NULL;
        }
        
        // Get the map from the current data
        const map_t *current_map = ar_data_get_map(current_data);
        if (!current_map) {
            free(segment);
            return NULL;
        }
        
        // Get the value for this segment
        const data_t *value = ar_map_get(current_map, segment);
        
        if (!value) {
            free(segment);
            return NULL;
        }
        
        // For all but the last segment, the value must be a map
        if (i < segment_count - 1 && value->type != DATA_MAP) {
            free(segment);
            return NULL;
        }
        
        current_data = value;
        free(segment);
    }
    
    return current_data;
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
        return 0.0;
    }
    
    const data_t *value = ar_data_get_map_data(data, key);
    
    if (!value) {
        return 0.0;
    }
    
    return ar_data_get_double(value);
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
 * Helper function to ensure that all intermediate maps in a path exist
 * @param data Pointer to the root map data to start from
 * @param path The path to create/ensure
 * @param final_segment_index The index of the final segment where the value will be set
 * @return Pointer to the map where the final value should be set, or NULL on failure
 */
static map_t* ensure_path_exists(data_t *data, const char *path, size_t *final_segment_index) {
    if (!data || !path || data->type != DATA_MAP) {
        return NULL;
    }
    
    // Check if the path contains any dots
    if (strchr(path, '.') == NULL) {
        // No dots, just use the current map
        *final_segment_index = 0;
        return data->data.map_ref;
    }
    
    // Count path segments for multi-segment paths
    size_t segment_count = ar_string_path_count(path, '.');
    if (segment_count == 0) {
        return NULL;
    }
    
    // Set the final segment index for the caller
    *final_segment_index = segment_count - 1;
    
    // Start with the current data's map
    map_t *current_map = data->data.map_ref;
    data_t *current_data = data;
    
    // Process each segment except the last one
    for (size_t i = 0; i < segment_count - 1; i++) {
        // Get the current segment
        char *segment = ar_string_path_segment(path, '.', i);
        if (!segment) {
            return NULL;
        }
        
        // Get the value for this segment
        data_t *value = ar_map_get(current_map, segment);
        
        // If the segment doesn't exist or isn't a map, create a new map
        if (!value || value->type != DATA_MAP) {
            // If there's an existing value but it's not a map, remove it
            if (value) {
                ar_data_destroy(value);
            }
            
            // Create a new map
            data_t *map_data = ar_data_create_map();
            if (!map_data) {
                free(segment);
                return NULL;
            }
            
            // Set the new map
            if (!ar_map_set(current_map, segment, map_data)) {
                ar_data_destroy(map_data);
                free(segment);
                return NULL;
            }
            
            value = map_data;
        }
        
        // Update current pointers
        current_data = value;
        current_map = current_data->data.map_ref;
        
        free(segment);
    }
    
    return current_map;
}

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
    
    // For simple keys with no dots, use direct access
    if (strchr(key, '.') == NULL) {
        map_t *map = data->data.map_ref;
        if (!map) {
            return false;
        }
        
        // Get the existing data for later cleanup
        data_t *prev_data = ar_map_get(map, key);
        
        // Create new data
        data_t *int_data = ar_data_create_integer(value);
        if (!int_data) {
            return false;
        }
        
        // Set the new value
        if (!ar_map_set(map, key, int_data)) {
            ar_data_destroy(int_data);
            return false;
        }
        
        // Free the old data after successful update
        if (prev_data) {
            ar_data_destroy(prev_data);
        }
        
        return true;
    }
    
    // Handle path-based access for keys with dots
    size_t final_segment_index = 0;
    map_t *target_map = ensure_path_exists(data, key, &final_segment_index);
    if (!target_map) {
        return false;
    }
    
    // Get the final segment key
    char *final_key = ar_string_path_segment(key, '.', final_segment_index);
    if (!final_key) {
        return false;
    }
    
    // Get the existing data for later cleanup
    data_t *prev_data = ar_map_get(target_map, final_key);
    
    // Create new data
    data_t *int_data = ar_data_create_integer(value);
    if (!int_data) {
        free(final_key);
        return false;
    }
    
    // Set the new value
    if (!ar_map_set(target_map, final_key, int_data)) {
        ar_data_destroy(int_data);
        free(final_key);
        return false;
    }
    
    // Free the old data after successful update
    if (prev_data) {
        ar_data_destroy(prev_data);
    }
    
    free(final_key);
    return true;
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
    
    // For simple keys with no dots, use direct access
    if (strchr(key, '.') == NULL) {
        map_t *map = data->data.map_ref;
        if (!map) {
            return false;
        }
        
        // Get the existing data for later cleanup
        data_t *prev_data = ar_map_get(map, key);
        
        // Create new data
        data_t *double_data = ar_data_create_double(value);
        if (!double_data) {
            return false;
        }
        
        // Set the new value
        if (!ar_map_set(map, key, double_data)) {
            ar_data_destroy(double_data);
            return false;
        }
        
        // Free the old data after successful update
        if (prev_data) {
            ar_data_destroy(prev_data);
        }
        
        return true;
    }
    
    // Handle path-based access for keys with dots
    size_t final_segment_index = 0;
    map_t *target_map = ensure_path_exists(data, key, &final_segment_index);
    if (!target_map) {
        return false;
    }
    
    // Get the final segment key
    char *final_key = ar_string_path_segment(key, '.', final_segment_index);
    if (!final_key) {
        return false;
    }
    
    // Get the existing data for later cleanup
    data_t *prev_data = ar_map_get(target_map, final_key);
    
    // Create new data
    data_t *double_data = ar_data_create_double(value);
    if (!double_data) {
        free(final_key);
        return false;
    }
    
    // Set the new value
    if (!ar_map_set(target_map, final_key, double_data)) {
        ar_data_destroy(double_data);
        free(final_key);
        return false;
    }
    
    // Free the old data after successful update
    if (prev_data) {
        ar_data_destroy(prev_data);
    }
    
    free(final_key);
    return true;
}

/**
 * Set a string value in a map data structure by key
 * @param data Pointer to the map data to modify
 * @param key The key or path to set in the map (supports "key.sub_key.sub_sub_key" format)
 * @param value The string value to store (will be copied)
 * @return true if successful, false if data is NULL, not a map, or allocation failure
 */
bool ar_data_set_map_string(data_t *data, const char *key, const char *value) {
    if (!data || !key || data->type != DATA_MAP) {
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
        
        // Create new data
        data_t *string_data = ar_data_create_string(value);
        if (!string_data) {
            return false;
        }
        
        // Set the new value
        if (!ar_map_set(map, key, string_data)) {
            ar_data_destroy(string_data);
            return false;
        }
        
        // Free the old data after successful update
        if (prev_data) {
            ar_data_destroy(prev_data);
        }
        
        return true;
    }
    
    // Handle path-based access for keys with dots
    size_t final_segment_index = 0;
    map_t *target_map = ensure_path_exists(data, key, &final_segment_index);
    if (!target_map) {
        return false;
    }
    
    // Get the final segment key
    char *final_key = ar_string_path_segment(key, '.', final_segment_index);
    if (!final_key) {
        return false;
    }
    
    // Get the existing data for later cleanup
    data_t *prev_data = ar_map_get(target_map, final_key);
    
    // Create new data
    data_t *string_data = ar_data_create_string(value);
    if (!string_data) {
        free(final_key);
        return false;
    }
    
    // Set the new value
    if (!ar_map_set(target_map, final_key, string_data)) {
        ar_data_destroy(string_data);
        free(final_key);
        return false;
    }
    
    // Free the old data after successful update
    if (prev_data) {
        ar_data_destroy(prev_data);
    }
    
    free(final_key);
    return true;
}
