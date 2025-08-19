#include "ar_yaml_reader.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>

/* Forward declaration of parse state */
typedef struct ar_yaml_parse_state_s ar_yaml_parse_state_t;

/* Opaque reader structure */
struct ar_yaml_reader_s {
    ar_log_t *ref_log;  /* Borrowed reference for error reporting */
    ar_yaml_parse_state_t *own_state;  /* Owned parse state (created/destroyed per read) */
};

/* Forward declarations for internal functions */
static int _get_indentation(const char *line);
static ar_data_t* _infer_scalar_type(const char *value);
static void _handle_list_item(ar_data_t *mut_list, const char *value);
static void _update_container_stack(ar_yaml_parse_state_t *mut_state, int indent);
static void _add_map_value(ar_data_t *mut_map, const char *key, const char *value);
static bool _should_skip_line(const char *line);

/**
 * Parse state for YAML reading
 */
struct ar_yaml_parse_state_s {
    ar_data_t *containers[100];  // Stack of open containers
    int depths[100];             // Indentation depth of each container
    int stack_size;              // Current stack depth
    char *current_key;           // Current key for map values
};

/**
 * Get indentation level (count leading spaces)
 */
static int _get_indentation(const char *line) {
    int indent = 0;
    while (*line == ' ') {
        indent++;
        line++;
    }
    return indent / 2;  // 2 spaces per level
}

/**
 * Create a new YAML reader instance
 */
ar_yaml_reader_t* ar_yaml_reader__create(ar_log_t *ref_log) {
    ar_yaml_reader_t *own_reader = AR__HEAP__MALLOC(sizeof(ar_yaml_reader_t), "ar_yaml_reader_t");
    if (own_reader == NULL) {
        return NULL;
    }
    
    own_reader->ref_log = ref_log;
    own_reader->own_state = NULL;  /* Will be created during read operations */
    return own_reader;
}

/**
 * Destroy a YAML reader instance
 */
void ar_yaml_reader__destroy(ar_yaml_reader_t *own_reader) {
    if (own_reader == NULL) {
        return;
    }
    
    AR__HEAP__FREE(own_reader);
}

/**
 * Read YAML file into ar_data_t structure
 */
ar_data_t* ar_yaml_reader__read_from_file(ar_yaml_reader_t *mut_reader, const char *ref_filename) {
    (void)mut_reader; // Will be used in later cycles
    if (!ref_filename) {
        return NULL;
    }
    
    FILE *file = fopen(ref_filename, "r");
    if (!file) {
        return NULL;
    }
    
    // Skip header line if present  
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return NULL;
    }
    
    // Check if header indicates a simple value
    if (strncmp(buffer, "# AgeRun YAML", 13) == 0) {
        // Read the next line to determine type (skip comments)
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            // Remove trailing newline
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
                len--;
            }
            
            // Skip empty lines and comments
            if (_should_skip_line(buffer)) {
                continue;
            }
            
            // Found first non-comment line
            break;
        }
        
        // Check for empty containers
        if (strcmp(buffer, "{}") == 0) {
            // Empty map
            fclose(file);
            return ar_data__create_map();
        } else if (strcmp(buffer, "[]") == 0) {
            // Empty list
            fclose(file);
            return ar_data__create_list();
        }
        // Check if it's a list (starts with "-"), map (has ":"), or simple value
        else if (buffer[0] == '-' && buffer[1] == ' ') {
            // It's a list - rewind to process this line
            fseek(file, -(long)strlen(buffer) - 1, SEEK_CUR);
            
            // Create root list and use general parsing
            ar_data_t *own_root = ar_data__create_list();
            ar_yaml_parse_state_t state = {0};
            state.containers[0] = own_root;
            state.depths[0] = 0;
            state.stack_size = 1;
            
            // Parse using the general logic
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                size_t item_len = strlen(buffer);
                if (item_len > 0 && buffer[item_len - 1] == '\n') {
                    buffer[item_len - 1] = '\0';
                    item_len--;
                }
                
                if (item_len == 0) continue;
                
                int indent = _get_indentation(buffer);
                char *content = buffer + (indent * 2);
                
                // Skip comments
                if (content[0] == '#') continue;
                
                // List item
                if (content[0] == '-' && content[1] == ' ') {
                    char *value = content + 2;
                    
                    // Update stack
                    _update_container_stack(&state, indent);
                    
                    // Check if map item
                    if (strchr(value, ':')) {
                        ar_data_t *own_map = ar_data__create_map();
                        ar_data__list_add_last_data(own_root, own_map);
                        
                        // Parse inline key:value
                        char *colon = strchr(value, ':');
                        *colon = '\0';
                        char *key = value;
                        char *val = colon + 1;
                        while (*val == ' ') val++;
                        
                        if (*val != '\0') {
                            _add_map_value(own_map, key, val);
                        }
                        
                        // Push map for subsequent keys
                        if (state.stack_size < 100) {
                            state.containers[state.stack_size] = own_map;
                            state.depths[state.stack_size] = indent + 1;
                            state.stack_size++;
                        }
                    } else {
                        _handle_list_item(own_root, value);
                    }
                }
                // Map key in list item
                else if (strchr(content, ':') && state.stack_size > 1) {
                    ar_data_t *mut_map = state.containers[state.stack_size - 1];
                    if (ar_data__get_type(mut_map) == AR_DATA_TYPE__MAP) {
                        char *colon = strchr(content, ':');
                        *colon = '\0';
                        char *key = content;
                        char *val = colon + 1;
                        while (*val == ' ') val++;
                        
                        if (*val != '\0') {
                            _add_map_value(mut_map, key, val);
                        }
                    }
                }
            }
            
            fclose(file);
            return own_root;
        } else if (strchr(buffer, ':') == NULL) {
            // Simple scalar value
            fclose(file);
            return ar_data__create_string(buffer);
        }
        
        // It's a map - rewind to process this line
        fseek(file, -(long)strlen(buffer) - 1, SEEK_CUR);
    }
    
    // Initialize parse state
    ar_yaml_parse_state_t state = {0};
    ar_data_t *own_root = ar_data__create_map();
    state.containers[0] = own_root;
    state.depths[0] = 0;
    state.stack_size = 1;
    state.current_key = NULL;
    
    // Parse line by line
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        
        // Skip empty lines
        if (len == 0) {
            continue;
        }
        
        int indent = _get_indentation(buffer);
        char *content = buffer + (indent * 2);
        
        // Skip comments (after getting indentation)
        if (content[0] == '#') {
            continue;
        }
        
        // Check for empty containers on their own line (for nested empty values)
        if ((strcmp(content, "[]") == 0 || strcmp(content, "{}") == 0) && 
            state.current_key && state.stack_size > 0) {
            ar_data_t *mut_parent = state.containers[state.stack_size - 1];
            if (ar_data__get_type(mut_parent) == AR_DATA_TYPE__MAP) {
                ar_data_t *own_empty = (strcmp(content, "[]") == 0) ? 
                    ar_data__create_list() : ar_data__create_map();
                ar_data__set_map_data(mut_parent, state.current_key, own_empty);
                AR__HEAP__FREE(state.current_key);
                state.current_key = NULL;
            }
        }
        // Check for list item
        else if (content[0] == '-' && content[1] == ' ') {
            char *value = content + 2;
            
            // If we were expecting a value for a map key, create a list
            if (state.current_key && state.stack_size > 0) {
                ar_data_t *mut_parent = state.containers[state.stack_size - 1];
                if (ar_data__get_type(mut_parent) == AR_DATA_TYPE__MAP) {
                    // Create new list for this key
                    ar_data_t *own_list = ar_data__create_list();
                    ar_data__set_map_data(mut_parent, state.current_key, own_list);
                    
                    // Push list onto stack
                    if (state.stack_size < 100) {
                        state.containers[state.stack_size] = own_list;
                        state.depths[state.stack_size] = indent;
                        state.stack_size++;
                    }
                    
                    AR__HEAP__FREE(state.current_key);
                    state.current_key = NULL;
                }
            }
            
            // Check if this list item is a map (has colon)
            if (strchr(value, ':')) {
                // Create a new map as list item
                ar_data_t *own_map = ar_data__create_map();
                
                // Add to current list
                if (state.stack_size > 0) {
                    ar_data_t *mut_list = state.containers[state.stack_size - 1];
                    if (ar_data__get_type(mut_list) == AR_DATA_TYPE__LIST) {
                        ar_data__list_add_last_data(mut_list, own_map);
                    }
                }
                
                // Parse the inline key:value pair
                char *colon = strchr(value, ':');
                *colon = '\0';
                char *key = value;
                char *val = colon + 1;
                while (*val == ' ') val++;
                
                if (*val != '\0') {
                    _add_map_value(own_map, key, val);
                }
                
                // Push map onto stack for subsequent keys
                if (state.stack_size < 100) {
                    state.containers[state.stack_size] = own_map;
                    state.depths[state.stack_size] = indent + 1;  // Next keys will be indented
                    state.stack_size++;
                }
            } else {
                // Simple scalar item
                if (state.stack_size > 0) {
                    ar_data_t *mut_list = state.containers[state.stack_size - 1];
                    if (ar_data__get_type(mut_list) == AR_DATA_TYPE__LIST) {
                        _handle_list_item(mut_list, value);
                    }
                }
            }
        }
        // Check for key:value pair  
        else if (strchr(content, ':')) {
            // Check if this is the first item of a nested map/list (we have a pending key and this line is indented)
            if (state.current_key && indent > state.depths[state.stack_size - 1]) {
                // Create a new map for the pending key
                ar_data_t *mut_parent = state.containers[state.stack_size - 1];
                ar_data_t *own_nested_map = ar_data__create_map();
                ar_data__set_map_data(mut_parent, state.current_key, own_nested_map);
                
                // Push the new map onto the stack
                if (state.stack_size < 100) {
                    state.containers[state.stack_size] = own_nested_map;
                    state.depths[state.stack_size] = indent;
                    state.stack_size++;
                }
                
                // Clear the pending key
                AR__HEAP__FREE(state.current_key);
                state.current_key = NULL;
                
                // Now process this line as a key:value in the new nested map
                char *colon = strchr(content, ':');
                *colon = '\0';
                char *key = content;
                char *value = colon + 1;
                while (*value == ' ') value++;
                
                if (*value == '\0') {
                    // Another nested structure
                    state.current_key = AR__HEAP__STRDUP(key, "yaml key");
                } else {
                    // Scalar value in nested map
                    _add_map_value(own_nested_map, key, value);
                }
            } else {
                // Regular key:value pair handling
                // Update stack based on indentation
                _update_container_stack(&state, indent);
                
                char *colon = strchr(content, ':');
                *colon = '\0';
                char *key = content;
                char *value = colon + 1;
                
                // Skip leading whitespace in value
                while (*value == ' ') value++;
                
                // Get current container
                ar_data_t *mut_container = state.containers[state.stack_size - 1];
                
                if (*value == '\0') {
                    // Empty value - next line will contain list or map
                    if (state.current_key) {
                        AR__HEAP__FREE(state.current_key);
                    }
                    state.current_key = AR__HEAP__STRDUP(key, "yaml key");
                } else if (strcmp(value, "[]") == 0) {
                    // Empty list
                    ar_data_t *own_empty_list = ar_data__create_list();
                    ar_data__set_map_data(mut_container, key, own_empty_list);
                } else if (strcmp(value, "{}") == 0) {
                    // Empty map
                    ar_data_t *own_empty_map = ar_data__create_map();
                    ar_data__set_map_data(mut_container, key, own_empty_map);
                } else {
                    // Scalar value
                    _add_map_value(mut_container, key, value);
                }
            }
        }
    }
    
    // Clean up
    if (state.current_key) {
        AR__HEAP__FREE(state.current_key);
    }
    
    fclose(file);
    return own_root;  // Ownership transferred to caller
}

/**
 * Infer the type of a scalar value from its string representation
 */
static ar_data_t* _infer_scalar_type(const char *value) {
    if (!value) {
        return ar_data__create_string("");
    }
    
    // Check for quoted string
    if (*value == '"') {
        // Remove quotes
        const char *start = value + 1;
        char *end_quote = strchr(start, '"');
        if (end_quote) {
            size_t len = (size_t)(end_quote - start);
            char *unquoted = AR__HEAP__MALLOC(len + 1, "yaml unquoted string");
            strncpy(unquoted, start, len);
            unquoted[len] = '\0';
            ar_data_t *own_result = ar_data__create_string(unquoted);
            AR__HEAP__FREE(unquoted);
            return own_result;
        }
        return ar_data__create_string(start);
    }
    
    // Try to parse as integer
    char *endptr;
    errno = 0;
    long int_val = strtol(value, &endptr, 10);
    if (errno == 0 && *endptr == '\0') {
        return ar_data__create_integer((int)int_val);
    }
    
    // Try to parse as double
    errno = 0;
    double dbl_val = strtod(value, &endptr);
    if (errno == 0 && *endptr == '\0') {
        return ar_data__create_double(dbl_val);
    }
    
    // Default to string
    return ar_data__create_string(value);
}

/**
 * Add an item to a list after inferring its type
 */
static void _handle_list_item(ar_data_t *mut_list, const char *value) {
    ar_data_t *own_item = _infer_scalar_type(value);
    
    if (ar_data__get_type(own_item) == AR_DATA_TYPE__STRING) {
        ar_data__list_add_last_string(mut_list, ar_data__get_string(own_item));
    } else if (ar_data__get_type(own_item) == AR_DATA_TYPE__INTEGER) {
        ar_data__list_add_last_integer(mut_list, ar_data__get_integer(own_item));
    } else if (ar_data__get_type(own_item) == AR_DATA_TYPE__DOUBLE) {
        ar_data__list_add_last_double(mut_list, ar_data__get_double(own_item));
    }
    
    ar_data__destroy(own_item);
}

/**
 * Update container stack based on indentation level
 */
static void _update_container_stack(ar_yaml_parse_state_t *mut_state, int indent) {
    // Pop containers if we've decreased indentation
    while (mut_state->stack_size > 1 && indent < mut_state->depths[mut_state->stack_size - 1]) {
        mut_state->stack_size--;
    }
}

/**
 * Add a key-value pair to a map, inferring the value type
 */
static void _add_map_value(ar_data_t *mut_map, const char *key, const char *value) {
    ar_data_t *own_value = _infer_scalar_type(value);
    if (ar_data__get_type(own_value) == AR_DATA_TYPE__STRING) {
        ar_data__set_map_string(mut_map, key, ar_data__get_string(own_value));
    } else if (ar_data__get_type(own_value) == AR_DATA_TYPE__INTEGER) {
        ar_data__set_map_integer(mut_map, key, ar_data__get_integer(own_value));
    } else if (ar_data__get_type(own_value) == AR_DATA_TYPE__DOUBLE) {
        ar_data__set_map_double(mut_map, key, ar_data__get_double(own_value));
    }
    ar_data__destroy(own_value);
}

/**
 * Check if a line should be skipped (empty or comment)
 */
static bool _should_skip_line(const char *line) {
    if (!line || *line == '\0') return true;
    
    // Skip leading whitespace to find comment
    while (*line == ' ' || *line == '\t') line++;
    
    return *line == '#';

}

