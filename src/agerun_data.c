#include "../include/agerun_data.h"
#include <stdlib.h>
#include <string.h>

/* Forward declaration for recursive freeing */
static void free_dict(dict_t *dict);

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
        case DATA_DICT:
            data.data.dict_value = ar_dict_create();
            break;
    }
    
    return data;
}

/**
 * Create a new empty dictionary
 * @return Pointer to the new dictionary, or NULL on failure
 */
dict_t* ar_dict_create(void) {
    dict_t *dict = (dict_t *)malloc(sizeof(dict_t));
    if (!dict) return NULL;
    
    if (!ar_dict_init(dict)) {
        free(dict);
        return NULL;
    }
    
    return dict;
}

/**
 * Initialize a dictionary
 * @param dict Dictionary to initialize
 * @return true if successful, false otherwise
 */
bool ar_dict_init(dict_t *dict) {
    if (!dict) {
        return false;
    }
    
    for (int i = 0; i < DICT_SIZE; i++) {
        dict->entries[i].is_used = false;
        dict->entries[i].key = NULL;
    }
    
    dict->count = 0;
    return true;
}

/**
 * Get a value from dictionary by key
 * @param dict Dictionary
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_dict_get(dict_t *dict, const char *key) {
    if (!dict || !key) {
        return NULL;
    }
    
    for (int i = 0; i < DICT_SIZE; i++) {
        if (dict->entries[i].is_used && dict->entries[i].key && 
            strcmp(dict->entries[i].key, key) == 0) {
            return &dict->entries[i].value;
        }
    }
    return NULL;
}

/**
 * Set a value in dictionary
 * @param dict Dictionary
 * @param key Key to set
 * @param value_ptr Pointer to value to set
 * @return true if successful, false otherwise
 */
bool ar_dict_set(dict_t *dict, const char *key, data_t *value_ptr) {
    if (!dict || !key || !value_ptr) {
        return false;
    }
    
    // First, check if key already exists using ar_dict_get
    data_t *existing = ar_dict_get(dict, key);
    if (existing) {
        // Free old value
        ar_free_data(existing);
        
        // Directly assign the new value
        *existing = *value_ptr;
        
        return true;
    }
    
    // Find empty slot
    for (int i = 0; i < DICT_SIZE; i++) {
        if (!dict->entries[i].is_used) {
            char *key_copy = strdup(key);
            if (!key_copy) {
                return false;
            }
            
            dict->entries[i].is_used = true;
            dict->entries[i].key = key_copy;
            dict->entries[i].value = *value_ptr;
            
            dict->count++;
            return true;
        }
    }
    
    return false; // No space left
}

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_free_data(data_t *data) {
    if (!data) return;
    
    if (data->type == DATA_STRING && data->data.string_value) {
        free(data->data.string_value);
        data->data.string_value = NULL;
    } else if (data->type == DATA_DICT && data->data.dict_value) {
        free_dict(data->data.dict_value);
        data->data.dict_value = NULL;
    }
}

/**
 * Free all resources in a dictionary
 * @param dict Dictionary to free
 */
static void free_dict(dict_t *dict) {
    if (!dict) return;
    
    for (int i = 0; i < DICT_SIZE; i++) {
        if (dict->entries[i].is_used && dict->entries[i].key) {
            free(dict->entries[i].key);
            ar_free_data(&dict->entries[i].value);
        }
    }
    
    /* Free the dictionary structure itself */
    free(dict);
}
