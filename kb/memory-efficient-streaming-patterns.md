# Memory-Efficient Streaming Patterns

## Learning
When implementing file I/O operations, avoid creating intermediate string representations that duplicate memory usage. Instead, stream data directly to/from files using FILE* operations. This pattern is especially critical for serialization formats like YAML where the temptation is to build complete strings in memory.

## Importance
Memory duplication through intermediate string representations doubles the memory footprint unnecessarily. For large data structures, this can cause significant performance degradation and memory pressure. Direct streaming eliminates this overhead while maintaining clean APIs.

## Example
**Initial design (memory-inefficient)**:
```c
// BAD: Creates intermediate string representation
// EXAMPLE: This shows what NOT to do - function does not exist in AgeRun
char* yaml_to_string_bad_example(const ar_data_t *ref_data) {
    // Builds entire YAML string in memory
    char *buffer = malloc(LARGE_SIZE);
    build_yaml_string(buffer, ref_data);  // EXAMPLE: Hypothetical function
    return buffer;  // Caller must free
}

bool yaml_write_inefficient(const ar_data_t *ref_data, const char *filename) {
    char *yaml_string = yaml_to_string_bad_example(ref_data);  // Duplicates memory
    FILE *file = fopen(filename, "w");
    fprintf(file, "%s", yaml_string);
    fclose(file);
    free(yaml_string);  // Extra allocation/deallocation
    return true;
}
```

**Improved design (memory-efficient)**:
```c
// GOOD: Streams directly to file
bool ar_yaml__write_to_file(const ar_data_t *ref_data, const char *ref_filename) {
    FILE *file = fopen(ref_filename, "w");
    if (!file) return false;
    
    _write_yaml_to_file(file, ref_data, 0, false);  // Direct streaming
    
    fclose(file);
    return true;
}

static void _write_yaml_to_file(FILE *file, const ar_data_t *ref_data, int indent_level, bool is_list_item) {
    // Write directly to FILE* without intermediate buffers
    switch (ar_data__get_type(ref_data)) {
        case AR_DATA_TYPE_INTEGER:
            fprintf(file, "%lld", ar_data__get_integer(ref_data));
            break;
        case AR_DATA_TYPE_STRING:
            fprintf(file, "\"%s\"", ar_data__get_string(ref_data));
            break;
        // ... other types stream directly
    }
}
```

## Generalization
The streaming pattern applies to any file I/O operation:
1. **Serialization**: Write data structures directly to files
2. **Deserialization**: Parse directly from file streams
3. **Transformation**: Process data in chunks rather than loading entirely
4. **Export/Import**: Stream between formats without full materialization

## Implementation Guidelines
```c
// EXAMPLE: Pattern for streaming complex structures
typedef struct {
    FILE *file;
    int indent_level;
    bool has_error;
    char error_message[256];
} streaming_context;  // EXAMPLE: Hypothetical type

// EXAMPLE: Conceptual streaming pattern
void stream_data_example(FILE *file, const ar_data_t *ref_data) {
    // Stream components directly based on type
    switch (ar_data__get_type(ref_data)) {
        case AR_DATA_TYPE__INTEGER:
            fprintf(file, "%d", ar_data__get_integer(ref_data));
            break;
        case AR_DATA_TYPE__STRING:
            fprintf(file, "%s", ar_data__get_string(ref_data));
            break;
        case AR_DATA_TYPE__LIST: {
            size_t count = ar_data__list_count(ref_data);
            ar_data_t **items = ar_data__list_items(ref_data);
            for (size_t i = 0; i < count; i++) {
                stream_data_example(file, items[i]);
            }
            break;
        }
        default:
            break;
    }
}

// Usage pattern with actual AgeRun functions
bool export_data_example(const ar_data_t *ref_data, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return false;
    
    stream_data_example(file, ref_data);
    
    fclose(file);
    return true;
}
```

## Benefits
- **Memory efficiency**: O(1) memory for serialization instead of O(n)
- **Performance**: Reduced allocations and copies
- **Scalability**: Can handle arbitrarily large data structures
- **Simplicity**: Fewer ownership transfers and cleanup requirements

## Static Analysis Considerations
When using direct file operations, ensure proper error checking:
```c
// Check all stream operations for static analysis compliance
if (fgets(buffer, sizeof(buffer), file) == NULL) {
    if (!feof(file)) {
        // Handle error
    }
    // Handle EOF
}

if (fprintf(file, "%s", data) < 0) {
    // Handle write error
}
```

## Related Patterns
- [Static Analyzer Stream Compliance](static-analyzer-stream-compliance.md)
- [File I/O Backup Mechanism](file-io-backup-mechanism.md)
- [Error Propagation Pattern](error-propagation-pattern.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Resource-Aware API Design](resource-aware-api-design.md)

## AGENTS.md Reference
This pattern is referenced in [AGENTS.md](../AGENTS.md) under Memory Management practices.