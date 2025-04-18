#include "agerun_data.h"
#include "agerun_map.h"
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
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_data_free(data_t *data) {
    if (!data) return;
    
    if (data->type == DATA_STRING && data->data.string_value) {
        free(data->data.string_value);
        data->data.string_value = NULL;
    } else if (data->type == DATA_MAP && data->data.map_value) {
        ar_map_free(data->data.map_value);
        data->data.map_value = NULL;
    }
}
