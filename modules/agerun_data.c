#include "agerun_data.h"
#include <stdlib.h>
#include <string.h>

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
const map_t *ar_data_get_map(const data_t *data) {
    if (!data || data->type != DATA_MAP) {
        return NULL;
    }
    return data->data.map_ref;
}

/**
 * Get a mutable map value from a data structure
 * @param data Pointer to the data to retrieve from
 * @return The map value or NULL if data is NULL or not a map type
 */
map_t *ar_data_get_map_mutable(data_t *data) {
    if (!data || data->type != DATA_MAP) {
        return NULL;
    }
    return data->data.map_ref;
}

/**
 * Get an integer value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The integer value, or 0 if data is NULL, not a map, key not found, or value not an integer
 */
int ar_data_get_sub_integer(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return 0;
    }
    
    const map_t *map = ar_data_get_map(data);
    const data_t *value = (const data_t *)ar_map_get(map, key);
    
    if (!value) {
        return 0;
    }
    
    return ar_data_get_integer(value);
}

/**
 * Get a double value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The double value, or 0.0 if data is NULL, not a map, key not found, or value not a double
 */
double ar_data_get_sub_double(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return 0.0;
    }
    
    const map_t *map = ar_data_get_map(data);
    const data_t *value = (const data_t *)ar_map_get(map, key);
    
    if (!value) {
        return 0.0;
    }
    
    return ar_data_get_double(value);
}

/**
 * Get a string value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The string value, or NULL if data is NULL, not a map, key not found, or value not a string
 */
const char *ar_data_get_sub_string(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return NULL;
    }
    
    const map_t *map = ar_data_get_map(data);
    const data_t *value = (const data_t *)ar_map_get(map, key);
    
    if (!value) {
        return NULL;
    }
    
    return ar_data_get_string(value);
}

/**
 * Get a sub-map value from a map data structure by key
 * @param data Pointer to the map data to retrieve from
 * @param key The key to look up in the map
 * @return The map value, or NULL if data is NULL, not a map, key not found, or value not a map
 */
const map_t *ar_data_get_sub_map(const data_t *data, const char *key) {
    if (!data || !key || data->type != DATA_MAP) {
        return NULL;
    }
    
    const map_t *map = ar_data_get_map(data);
    const data_t *value = (const data_t *)ar_map_get(map, key);
    
    if (!value) {
        return NULL;
    }
    
    return ar_data_get_map(value);
}
