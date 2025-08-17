# Container Stack Parsing Pattern

## Learning
When parsing indentation-based formats like YAML, use a container stack to track open collections (maps/lists) at different depths. The stack maintains both the container reference and its indentation level, allowing correct parent-child relationships to be established as the parser processes lines at varying indentation levels.

## Importance
Without a proper container stack, parsers cannot correctly handle nested structures or determine where to add new elements when indentation changes. The stack provides context for understanding the hierarchical relationship between elements at different indentation levels.

## Example
```c
// Stack-based parsing state
typedef struct ar_yaml_parse_state_s {
    ar_data_t *containers[100];  // Stack of open containers
    int depths[100];             // Indentation depth of each container
    int stack_size;              // Current stack depth
    char *current_key;           // Key waiting for its value in a map
} ar_yaml_parse_state_t;

// Managing the container stack based on indentation
static void _update_container_stack(ar_yaml_parse_state_t *mut_state, int indent) {
    // Pop containers that are deeper than current line
    while (mut_state->stack_size > 0 && 
           mut_state->depths[mut_state->stack_size - 1] >= indent) {
        mut_state->stack_size--;
    }
}

// Adding to the current container
static void _add_to_current_container(ar_yaml_parse_state_t *mut_state, 
                                      const char *key, ar_data_t *own_value) {
    if (mut_state->stack_size == 0) return;
    
    ar_data_t *mut_container = mut_state->containers[mut_state->stack_size - 1];
    
    if (ar_data__get_type(mut_container) == AR_DATA_TYPE__LIST) {
        ar_data__list_add_last_data(mut_container, own_value);
    } else if (ar_data__get_type(mut_container) == AR_DATA_TYPE__MAP) {
        if (key) {
            ar_data__set_map_data(mut_container, key, own_value);
        }
    }
}

// Pushing new container onto stack
static void _push_container(ar_yaml_parse_state_t *mut_state, 
                           ar_data_t *ref_container, int depth) {
    if (mut_state->stack_size < 100) {
        mut_state->containers[mut_state->stack_size] = ref_container;
        mut_state->depths[mut_state->stack_size] = depth;
        mut_state->stack_size++;
    }
}
```

## Generalization
Container stack pattern for hierarchical parsing:
1. **Initialize root**: Start with root container at depth 0
2. **Track indentation**: Store depth with each container
3. **Pop on dedent**: Remove containers deeper than current line
4. **Push on indent**: Add new containers when nesting increases
5. **Current container**: Top of stack is where new elements go
6. **Handle transitions**: Manage key→value and parent→child relationships

## Implementation
```c
// Complete parsing loop with stack management
ar_data_t* parse_yaml_file(FILE *file) {
    ar_yaml_parse_state_t state = {0};
    ar_data_t *own_root = NULL;
    char line[4096];
    
    while (fgets(line, sizeof(line), file)) {
        if (_should_skip_line(line)) continue;
        
        int indent = _get_indentation(line);
        char *content = line + indent;
        
        // Update stack based on indentation
        _update_container_stack(&state, indent);
        
        // Detect line type and process
        if (strncmp(content, "- ", 2) == 0) {
            // List item
            if (state.stack_size == 0) {
                // Root is a list
                own_root = ar_data__create_list();
                _push_container(&state, own_root, 0);
            }
            
            // Parse list item content
            content += 2;
            if (strchr(content, ':')) {
                // Inline map in list
                ar_data_t *own_map = ar_data__create_map();
                _add_to_current_container(&state, NULL, own_map);
                _push_container(&state, own_map, indent + 2);
                // Parse as map entry...
            } else {
                // Simple list item
                ar_data_t *own_value = _parse_scalar(content);
                _add_to_current_container(&state, NULL, own_value);
            }
        } else if (strchr(content, ':')) {
            // Map entry
            if (state.stack_size == 0) {
                // Root is a map
                own_root = ar_data__create_map();
                _push_container(&state, own_root, 0);
            }
            // Parse key:value...
        }
    }
    
    // Cleanup
    AR__HEAP__FREE(state.current_key);
    return own_root;
}
```

## Stack Depth Edge Cases
```c
// Important: Compare depth with < not <=
// This ensures parent container stays on stack
while (state->stack_size > 0 && 
       state->depths[state->stack_size - 1] >= indent) {  // >= not >
    state->stack_size--;
}

// Example YAML and stack states:
// agents:           [root_map(0)]
//   - name: Alice   [root_map(0), list(2)]
//     age: 30       [root_map(0), list(2), map(4)]
//   - name: Bob     [root_map(0), list(2)]  <- map(4) popped
//     age: 25       [root_map(0), list(2), map(4)]
```

## Related Patterns
- [YAML Indentation Consistency Pattern](yaml-indentation-consistency-pattern.md)
- [Test Complexity Reduction Pattern](test-complexity-reduction-pattern.md)
- [YAML Serialization Direct I/O Pattern](yaml-serialization-direct-io-pattern.md)