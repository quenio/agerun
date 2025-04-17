#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>

/* Constants */
#define MEMORY_SIZE 256

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING
} data_type_t;

/**
 * Data structure for storing various data types
 */
typedef struct data_s {
    data_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
    } data;
} data_t;

/**
 * Memory Dictionary Entry for storing key-value pairs
 */
typedef struct memory_entry_s {
    char *key;
    data_t value;
    bool is_used;
} memory_entry_t;

/**
 * Memory Dictionary for storing agent state
 */
typedef struct memory_dict_s {
    memory_entry_t entries[MEMORY_SIZE];
    int count;
} memory_dict_t;

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_free_data(data_t *data);

/**
 * Get a value from memory dictionary by key
 * @param memory Memory dictionary
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_memory_get(void *memory, const char *key);

/**
 * Initialize a memory dictionary
 * @param dict Memory dictionary to initialize
 * @return true if successful, false otherwise
 */
bool ar_init_memory_dict(memory_dict_t *dict);

/**
 * Set a value in memory dictionary
 * @param memory Memory dictionary
 * @param key Key to set
 * @param value_ptr Pointer to value to set
 * @return true if successful, false otherwise
 */
bool ar_memory_set(void *memory, const char *key, void *value_ptr);

#endif /* AGERUN_DATA_H */
