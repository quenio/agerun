#include "ar_yaml_writer.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

/* Opaque writer structure */
struct ar_yaml_writer_s {
    ar_log_t *ref_log;  /* Borrowed reference for error reporting */
};

/* Forward declarations for internal functions */
static void _write_yaml_to_file(FILE *file, const ar_data_t *ref_data, int indent_level, bool is_list_item);
static void _write_indent(FILE *file, int indent_level);

/**
 * Create a new YAML writer instance
 */
ar_yaml_writer_t* ar_yaml_writer__create(ar_log_t *ref_log) {
    ar_yaml_writer_t *own_writer = AR__HEAP__MALLOC(sizeof(ar_yaml_writer_t), "ar_yaml_writer_t");
    if (own_writer == NULL) {
        return NULL;
    }
    
    own_writer->ref_log = ref_log;  /* Borrowed reference - NOT owned */
    return own_writer;  /* Ownership transferred to caller */
}

/**
 * Destroy a YAML writer instance
 */
void ar_yaml_writer__destroy(ar_yaml_writer_t *own_writer) {
    if (own_writer == NULL) {
        return;
    }
    
    /* Note: ref_log is a borrowed reference - do NOT destroy it */
    
    AR__HEAP__FREE(own_writer);
}

/**
 * Write indentation to file
 */
static void _write_indent(FILE *file, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        fprintf(file, "  ");
    }
}

/**
 * Write ar_data_t to YAML file recursively
 */
static void _write_yaml_to_file(FILE *file, const ar_data_t *ref_data, int indent_level, bool is_list_item) {
    (void)is_list_item; // Will be used for more complex formatting later
    
    if (!ref_data) {
        fprintf(file, "null");
        return;
    }
    
    ar_data_type_t type = ar_data__get_type(ref_data);
    
    switch (type) {
        case AR_DATA_TYPE__STRING: {
            const char *str = ar_data__get_string(ref_data);
            // Quote strings that contain special characters or could be misinterpreted
            if (str && (strchr(str, ':') || strchr(str, '\n') || strchr(str, '#'))) {
                fprintf(file, "\"%s\"", str);
            } else {
                fprintf(file, "%s", str ? str : "");
            }
            break;
        }
        
        case AR_DATA_TYPE__INTEGER: {
            int64_t value = ar_data__get_integer(ref_data);
            fprintf(file, "%" PRId64, value);
            break;
        }
        
        case AR_DATA_TYPE__DOUBLE: {
            fprintf(file, "%g", ar_data__get_double(ref_data));
            break;
        }
        
        case AR_DATA_TYPE__MAP: {
            ar_data_t *own_keys = ar_data__get_map_keys(ref_data);
            if (!own_keys) {
                fprintf(file, "{}");
                break;
            }
            
            size_t key_count = ar_data__list_count(own_keys);
            if (key_count == 0) {
                fprintf(file, "{}");
                ar_data__destroy(own_keys);
                break;
            }
            
            ar_data_t **keys = ar_data__list_items(own_keys);
            
            // For maps that are list items, format inline on first line
            bool first_in_list = is_list_item;
            
            for (size_t i = 0; i < key_count; i++) {
                if (!keys || !keys[i]) continue;
                
                const char *key_str = ar_data__get_string(keys[i]);
                if (!key_str) continue;
                
                // Write indentation (except for first item in list which is inline)
                if (first_in_list && i == 0) {
                    // First key-value pair goes on same line as list marker
                    first_in_list = false;
                } else {
                    if (i > 0) {
                        fprintf(file, "\n");
                    }
                    _write_indent(file, indent_level);
                }
                
                // Write key
                fprintf(file, "%s: ", key_str);
                
                // Write value
                ar_data_t *ref_value = ar_data__get_map_data(ref_data, key_str);
                
                // Check if value is a complex type that needs its own line
                ar_data_type_t value_type = ref_value ? ar_data__get_type(ref_value) : AR_DATA_TYPE__INTEGER;
                bool is_complex = (value_type == AR_DATA_TYPE__MAP || value_type == AR_DATA_TYPE__LIST);
                
                if (is_complex && value_type == AR_DATA_TYPE__LIST) {
                    // Lists need to be on new line - list writer will handle indentation
                    fprintf(file, "\n");
                    _write_yaml_to_file(file, ref_value, indent_level + 1, false);
                } else if (is_complex && value_type == AR_DATA_TYPE__MAP) {
                    // Maps continue on next line - use regular recursive approach
                    fprintf(file, "\n");
                    _write_yaml_to_file(file, ref_value, indent_level + 1, false);
                } else {
                    _write_yaml_to_file(file, ref_value, indent_level + 1, false);
                }
            }
            
            if (keys) {
                AR__HEAP__FREE(keys);
            }
            ar_data__destroy(own_keys);
            break;
        }
        
        case AR_DATA_TYPE__LIST: {
            size_t count = ar_data__list_count(ref_data);
            if (count == 0) {
                fprintf(file, "[]");
                break;
            }
            
            ar_data_t **items = ar_data__list_items(ref_data);
            
            for (size_t i = 0; i < count; i++) {
                if (!items || !items[i]) continue;
                
                if (i > 0) {
                    fprintf(file, "\n");
                }
                _write_indent(file, indent_level);
                fprintf(file, "- ");
                
                // Write the item
                _write_yaml_to_file(file, items[i], indent_level + 1, true);
            }
            
            if (items) {
                AR__HEAP__FREE(items);
            }
            break;
        }
        
        default:
            fprintf(file, "null");
            break;
    }
}

/**
 * Write ar_data_t structure to YAML file
 */
bool ar_yaml_writer__write_to_file(ar_yaml_writer_t *mut_writer, 
                                    const ar_data_t *ref_data, 
                                    const char *ref_filename) {
    if (!mut_writer || !ref_data || !ref_filename) {
        if (mut_writer && mut_writer->ref_log) {
            if (!ref_data) {
                ar_log__error(mut_writer->ref_log, "NULL data provided to YAML writer");
            } else if (!ref_filename) {
                ar_log__error(mut_writer->ref_log, "NULL filename provided to YAML writer");
            }
        }
        return false;
    }
    
    FILE *file = fopen(ref_filename, "w");
    if (!file) {
        if (mut_writer->ref_log) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Failed to open file for writing: %s", ref_filename);
            ar_log__error(mut_writer->ref_log, error_msg);
        }
        return false;
    }
    
    // Write header comment
    fprintf(file, "# AgeRun YAML File\n");
    
    // Write the data structure
    _write_yaml_to_file(file, ref_data, 0, false);
    fprintf(file, "\n");
    
    fclose(file);
    return true;
}

