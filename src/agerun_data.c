#include "../include/agerun_data.h"
#include <stdlib.h>
#include <string.h>

/* Forward declaration for recursive freeing */
static void free_dict(dict_t *dict);

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
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (dict->entries[i].is_used && dict->entries[i].key) {
            free(dict->entries[i].key);
            ar_free_data(&dict->entries[i].value);
        }
    }
    
    /* Free the dictionary structure itself */
    free(dict);
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
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        dict->entries[i].is_used = false;
        dict->entries[i].key = NULL;
    }
    
    dict->count = 0;
    return true;
}

/**
 * Get a value from dictionary by key
 * @param dictionary Dictionary
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_dict_get(void *dictionary, const char *key) {
    dict_t *dict = (dict_t *)dictionary;
    if (!dict || !key) {
        return NULL;
    }
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (dict->entries[i].is_used && dict->entries[i].key && 
            strcmp(dict->entries[i].key, key) == 0) {
            return &dict->entries[i].value;
        }
    }
    return NULL;
}

/**
 * Set a value in dictionary
 * @param dictionary Dictionary
 * @param key Key to set
 * @param value_ptr Pointer to value to set
 * @return true if successful, false otherwise
 */
bool ar_dict_set(void *dictionary, const char *key, void *value_ptr) {
    dict_t *dict = (dict_t *)dictionary;
    data_t value = *(data_t *)value_ptr;
    if (!dict || !key) {
        return false;
    }
    
    // First, check if key already exists using ar_dict_get
    data_t *existing = ar_dict_get(dict, key);
    if (existing) {
        // Free old value if it's a string
        ar_free_data(existing);
        
        // Set new value
        *existing = value;
        if (value.type == DATA_STRING && value.data.string_value) {
            existing->data.string_value = strdup(value.data.string_value);
        }
        
        return true;
    }
    
    // Find empty slot
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (!dict->entries[i].is_used) {
            dict->entries[i].is_used = true;
            dict->entries[i].key = strdup(key);
            dict->entries[i].value = value;
            
            if (value.type == DATA_STRING && value.data.string_value) {
                dict->entries[i].value.data.string_value = strdup(value.data.string_value);
            }
            
            dict->count++;
            return true;
        }
    }
    
    return false; // No space left
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
 * Create a new dictionary data value
 * @return Data value containing a dictionary
 */
data_t ar_data_create_dict(void) {
    data_t data;
    data.type = DATA_DICT;
    data.data.dict_value = ar_dict_create();
    return data;
}
