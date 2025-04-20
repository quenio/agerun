#include "agerun_data.h"
#include <stdlib.h>
#include <string.h>

/**
 * Create a new data value of the specified type with default value
 * @param type Type of data to create
 * @return Data value of the requested type
 */
data_t ar_data_create(data_type_t type) {
    data_t data;
    data.type = type;
    
    switch (type) {
        case DATA_INT:
            data.data.int_value = 0;
            break;
        case DATA_DOUBLE:
            data.data.double_value = 0.0;
            break;
        case DATA_STRING:
            data.data.string_value = NULL;
            break;
        case DATA_MAP:
            data.data.map_value = ar_map_create();
            break;
    }
    
    return data;
}

/**
 * Create a new integer data value
 * @param value Integer value to initialize with
 * @return Data value containing the integer
 */
data_t ar_data_create_integer(int64_t value) {
    data_t data;
    data.type = DATA_INT;
    data.data.int_value = value;
    return data;
}

/**
 * Create a new double data value
 * @param value Double value to initialize with
 * @return Data value containing the double
 */
data_t ar_data_create_double(double value) {
    data_t data;
    data.type = DATA_DOUBLE;
    data.data.double_value = value;
    return data;
}

/**
 * Create a new string data value
 * @param value String value to initialize with (will be copied)
 * @return Data value containing the string
 */
data_t ar_data_create_string(const char *value) {
    data_t data;
    data.type = DATA_STRING;
    data.data.string_value = value ? strdup(value) : NULL;
    return data;
}

/**
 * Create a new map data value
 * @return Data value containing an empty map
 */
data_t ar_data_create_map(void) {
    data_t data;
    data.type = DATA_MAP;
    data.data.map_value = ar_map_create();
    return data;
}

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_data_free(data_t *data) {
    if (!data) return;
    
    if (data->type == DATA_STRING && data->data.string_value) {
        free(data->data.string_value);
        data->data.string_value = NULL;
    } else if (data->type == DATA_MAP && data->data.map_value) {
        ar_map_destroy(data->data.map_value);
        data->data.map_value = NULL;
    }
}
