#ifndef AGERUN_DATA_H
#define AGERUN_DATA_H

#include <stdint.h>
#include <stdbool.h>

/* Constants */
#define DICT_SIZE 256

/**
 * Data type enumeration
 */
typedef enum {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_DICT
} data_type_t;

/* Forward declaration */
struct dict_s;

/**
 * Data structure for storing various data types
 */
typedef struct data_s {
    data_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
        struct dict_s *dict_value;
    } data;
} data_t;

/**
 * Dictionary Entry for storing key-value pairs
 */
typedef struct entry_s {
    char *key;
    data_t value;
    bool is_used;
} entry_t;

/**
 * Dictionary for storing agent state
 */
typedef struct dict_s {
    entry_t entries[DICT_SIZE];
    int count;
} dict_t;

/**
 * Free resources associated with a data structure
 * @param data Pointer to the data to free
 */
void ar_free_data(data_t *data);

/**
 * Get a value from dictionary by key
 * @param dict Dictionary
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_dict_get(dict_t *dict, const char *key);

/**
 * Initialize a dictionary
 * @param dict Dictionary to initialize
 * @return true if successful, false otherwise
 */
bool ar_dict_init(dict_t *dict);

/**
 * Set a value in dictionary (reference assignment)
 * @param dict Dictionary
 * @param key Key to set
 * @param value_ptr Pointer to value to set (value is referenced, not copied)
 * @return true if successful, false otherwise
 */
bool ar_dict_set(dict_t *dict, const char *key, data_t *value_ptr);

/**
 * Create a new empty dictionary
 * @return Pointer to the new dictionary, or NULL on failure
 */
dict_t* ar_dict_create(void);

/**
 * Create a new dictionary data value
 * @return Data value containing a dictionary
 */
data_t ar_data_create_dict(void);

#endif /* AGERUN_DATA_H */
