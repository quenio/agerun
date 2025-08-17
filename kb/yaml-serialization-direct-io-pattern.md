# YAML Serialization Direct I/O Pattern

## Learning
When implementing YAML serialization for data structures, design the module to work directly with files rather than strings. This avoids memory duplication and provides a cleaner API. The ar_data_t structure can serve as the intermediate representation for YAML nodes, eliminating the need for custom YAML-specific types.

## Importance
YAML serialization is commonly needed for configuration and persistence. A well-designed YAML module that works directly with ar_data_t structures provides maximum flexibility while avoiding the complexity of custom node types and string conversions.

## Example
**Module Design**:
```c
// ar_yaml.h - Clean, focused interface
bool ar_yaml__write_to_file(const ar_data_t *ref_data, const char *ref_filename);
ar_data_t* ar_yaml__read_from_file(const char *ref_filename);

// Note: No string conversion functions needed
// BAD: ar_yaml__to_string() - creates memory overhead
// BAD: ar_yaml__from_string() - requires intermediate allocation
```

**Implementation for complex structures**:
```c
static void _write_yaml_to_file(FILE *file, const ar_data_t *ref_data, int indent_level, bool is_list_item) {
    const char *indent = "  ";
    
    switch (ar_data__get_type(ref_data)) {
        case AR_DATA_TYPE__MAP: {
            ar_data_t *keys = ar_data__get_map_keys(ref_data);
            size_t count = ar_data__list_count(keys);
            ar_data_t **key_items = ar_data__list_items(keys);
            
            for (size_t i = 0; i < count; i++) {
                const char *key = ar_data__get_string(key_items[i]);
                
                // YAML formatting: first key inline with dash for list items
                if (i == 0 && is_list_item) {
                    fprintf(file, "%s: ", key);
                } else {
                    for (int j = 0; j < indent_level; j++) {
                        fprintf(file, "%s", indent);
                    }
                    fprintf(file, "%s: ", key);
                }
                
                ar_data_t *value = ar_data__get_map_data(ref_data, key);
                if (ar_data__get_type(value) == AR_DATA_TYPE__MAP || 
                    ar_data__get_type(value) == AR_DATA_TYPE__LIST) {
                    fprintf(file, "\n");
                    _write_yaml_to_file(file, value, indent_level + 1, false);
                } else {
                    _write_yaml_to_file(file, value, 0, false);
                    fprintf(file, "\n");
                }
            }
            ar_data__destroy(keys);
            break;
        }
        case AR_DATA_TYPE__LIST: {
            size_t count = ar_data__list_count(ref_data);
            ar_data_t **items = ar_data__list_items(ref_data);
            for (size_t i = 0; i < count; i++) {
                for (int j = 0; j < indent_level; j++) {
                    fprintf(file, "%s", indent);
                }
                fprintf(file, "- ");
                
                _write_yaml_to_file(file, items[i], indent_level + 1, true);
                if (ar_data__get_type(items[i]) != AR_DATA_TYPE__MAP) {
                    fprintf(file, "\n");
                }
            }
            break;
        }
        // ... handle primitive types
    }
}
```

## Generalization
Design principles for serialization modules:
1. **Use existing data structures**: Leverage ar_data_t instead of custom node types
2. **File-first API**: Design for file I/O, not string manipulation
3. **Recursive streaming**: Handle nested structures through recursion with context
4. **Format-specific logic**: Encapsulate formatting rules (indentation, markers) internally

## Implementation Strategy
```c
// Reading pattern (to be implemented)
ar_data_t* ar_yaml__read_from_file(const char *ref_filename) {
    FILE *file = fopen(ref_filename, "r");
    if (!file) return NULL;
    
    // EXAMPLE: Conceptual parser context
    struct {
        FILE *file;
        int line_number;
        int current_indent;
    } ctx = {
        .file = file,
        .line_number = 1,
        .current_indent = 0
    };
    
    ar_data_t *own_result = ar_data__create_string("parsed content");  // Simplified
    
    fclose(file);
    return own_result;  // Ownership transferred to caller
}

// Integration with persistence
bool ar_agent_store__save_with_instance(ar_agent_store_t *ref_store, ar_methodology_t *ref_methodology) {
    // EXAMPLE: Conceptual implementation
    ar_data_t *own_data = ar_data__create_map();  // Build agency data
    bool success = ar_yaml__write_to_file(own_data, "agerun.agency");
    ar_data__destroy(own_data);
    return success;
}
```

## Format Considerations
YAML-specific formatting rules:
- Lists: `- ` prefix with proper indentation
- Maps: `key: value` with colon separator
- Nested structures: 2-space indentation per level
- Inline first key: For list items containing maps
- String escaping: Quote strings containing special characters

## Testing Strategy
```c
void test_yaml__round_trip() {
    // Create complex nested structure
    ar_data_t *own_original = ar_data__create_map();
    ar_data_t *own_agents = ar_data__create_list();
    ar_data__set_map_data(own_original, "agents", own_agents);
    // ... build structure
    
    // Write to file
    assert(ar_yaml__write_to_file(own_original, "test.yaml"));
    
    // Read back
    ar_data_t *own_loaded = ar_yaml__read_from_file("test.yaml");
    assert(own_loaded != NULL);
    
    // Verify equivalence (conceptual - no equals function exists)
    // Would need to manually compare structures
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test.yaml");
}
```

## Related Patterns
- [Memory-Efficient Streaming Patterns](memory-efficient-streaming-patterns.md)
- [Multi-Line Data Persistence Format](multi-line-data-persistence-format.md)
- [YAML String Matching Pitfalls](yaml-string-matching-pitfalls.md)
- [Test File Cleanup Pattern](test-file-cleanup-pattern.md)
- [YAML Indentation Consistency Pattern](yaml-indentation-consistency-pattern.md)
- [Container Stack Parsing Pattern](container-stack-parsing-pattern.md)
- [Test Complexity Reduction Pattern](test-complexity-reduction-pattern.md)

## CLAUDE.md Reference
This pattern is referenced in [CLAUDE.md](../CLAUDE.md) under Development Practices for YAML serialization.