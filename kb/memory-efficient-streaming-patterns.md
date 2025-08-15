# Memory-Efficient Streaming Patterns

## Learning
When implementing file I/O operations, avoid creating intermediate string representations that duplicate memory usage. Instead, stream data directly to/from files using FILE* operations. This pattern is especially critical for serialization formats like YAML where the temptation is to build complete strings in memory.

## Importance
Memory duplication through intermediate string representations doubles the memory footprint unnecessarily. For large data structures, this can cause significant performance degradation and memory pressure. Direct streaming eliminates this overhead while maintaining clean APIs.

## Example
**Initial design (memory-inefficient)**:
```c
// BAD: Creates intermediate string representation
char* ar_yaml__to_string(const ar_data_t *ref_data) {
    // Builds entire YAML string in memory
    char *buffer = malloc(LARGE_SIZE);
    _build_yaml_string(buffer, ref_data);
    return buffer;  // Caller must free
}

bool ar_yaml__write_to_file(const ar_data_t *ref_data, const char *filename) {
    char *yaml_string = ar_yaml__to_string(ref_data);  // Duplicates memory
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
    FILE *file = ar_io__open_file(ref_filename, "w");
    if (!file) return false;
    
    _write_yaml_to_file(file, ref_data, 0, false);  // Direct streaming
    
    if (ar_io__close_file(file) != 0) {
        return false;
    }
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
// Pattern for streaming complex structures
typedef struct {
    FILE *file;
    int indent_level;
    bool has_error;
    char error_message[256];
} stream_context_t;

void stream_data_structure(stream_context_t *ctx, const void *data) {
    // Stream components directly
    if (fprintf(ctx->file, "header: %s\n", get_header(data)) < 0) {
        ctx->has_error = true;
        return;
    }
    
    // Stream collections iteratively
    for (size_t i = 0; i < get_count(data); i++) {
        stream_item(ctx, get_item(data, i));
        if (ctx->has_error) return;
    }
}

// Usage pattern
bool export_to_file(const void *data, const char *filename) {
    stream_context_t ctx = {
        .file = ar_io__open_file(filename, "w"),
        .indent_level = 0,
        .has_error = false
    };
    
    if (!ctx.file) return false;
    
    stream_data_structure(&ctx, data);
    
    bool success = !ctx.has_error && (ar_io__close_file(ctx.file) == 0);
    return success;
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

## CLAUDE.md Reference
This pattern is referenced in [CLAUDE.md](../CLAUDE.md) under Memory Management practices.