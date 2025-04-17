#include "../include/agerun_data.h"
#include <stdlib.h>
#include <string.h>

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_free_data(data_t *data) {
    if (!data) return;
    
    if (data->type == DATA_STRING && data->data.string_value) {
        free(data->data.string_value);
        data->data.string_value = NULL;
    }
}

/**
 * Initialize a memory dictionary
 * @param dict Memory dictionary to initialize
 * @return true if successful, false otherwise
 */
bool ar_init_memory_dict(memory_dict_t *dict) {
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
 * Get a value from memory dictionary by key
 * @param memory Memory dictionary
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_memory_get(void *memory, const char *key) {
    memory_dict_t *mem = (memory_dict_t *)memory;
    if (!mem || !key) {
        return NULL;
    }
    
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (mem->entries[i].is_used && mem->entries[i].key && 
            strcmp(mem->entries[i].key, key) == 0) {
            return &mem->entries[i].value;
        }
    }
    return NULL;
}

/**
 * Set a value in memory dictionary
 * @param memory Memory dictionary
 * @param key Key to set
 * @param value_ptr Pointer to value to set
 * @return true if successful, false otherwise
 */
bool ar_memory_set(void *memory, const char *key, void *value_ptr) {
    memory_dict_t *mem = (memory_dict_t *)memory;
    data_t value = *(data_t *)value_ptr;
    if (!mem || !key) {
        return false;
    }
    
    // First, check if key already exists using ar_memory_get
    data_t *existing = ar_memory_get(mem, key);
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
        if (!mem->entries[i].is_used) {
            mem->entries[i].is_used = true;
            mem->entries[i].key = strdup(key);
            mem->entries[i].value = value;
            
            if (value.type == DATA_STRING && value.data.string_value) {
                mem->entries[i].value.data.string_value = strdup(value.data.string_value);
            }
            
            mem->count++;
            return true;
        }
    }
    
    return false; // No space left
}
