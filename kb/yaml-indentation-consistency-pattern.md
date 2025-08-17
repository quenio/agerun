# YAML Indentation Consistency Pattern

## Learning
YAML readers and writers must use identical indentation rules to ensure round-trip compatibility. When a writer uses inconsistent indentation (mixing 2 and 4 spaces), the reader's stack-based parsing can fail to correctly reconstruct nested structures.

## Importance
Inconsistent indentation between writer and reader is a common source of YAML parsing failures. It causes nested structures to be incorrectly parsed, leading to data loss or corruption. Establishing a single indentation standard (2 spaces) and using it consistently throughout both writing and reading ensures reliable serialization.

## Example
**Problem: Inconsistent writer indentation**
```c
// BAD: Writer mixing indentation levels
static void _write_map_entry(FILE *file, const char *key, int indent, bool first_in_list) {
    if (first_in_list) {
        fprintf(file, "%s: ", key);  // No indentation for first key
    } else {
        // Mixing 2-space and 4-space indentation!
        for (int i = 0; i < indent; i++) {
            fprintf(file, "    ");  // 4 spaces per level
        }
        fprintf(file, "%s: ", key);
    }
}
```

**Solution: Consistent 2-space indentation**
```c
// GOOD: Consistent indentation throughout
static void _write_yaml_recursive(FILE *file, const ar_data_t *data, int depth) {
    const char *indent = "  ";  // Always 2 spaces
    
    // Apply consistent indentation
    for (int i = 0; i < depth; i++) {
        fprintf(file, "%s", indent);
    }
    
    // Write content based on type
    switch (ar_data__get_type(data)) {
        case AR_DATA_TYPE__MAP:
            _write_map_recursive(file, data, depth);
            break;
        // ...
    }
}
```

## Generalization
YAML indentation rules for consistent parsing:
1. **Choose one standard**: 2 spaces is most common in YAML
2. **Apply everywhere**: Both writer and reader must use the same
3. **Track depth consistently**: Increment/decrement by 1 for each nesting level
4. **Special cases**: Handle list items with maps (inline first key) consistently
5. **Test round-trips**: Verify write→read→write produces identical output

## Implementation
```c
// Parser stack management aligned with writer indentation
typedef struct {
    ar_data_t *containers[100];  // Stack of open containers
    int depths[100];             // Indentation depth (in 2-space units)
    int stack_size;              // Current stack depth
} ar_yaml_parse_state_t;

static void _update_container_stack(ar_yaml_parse_state_t *state, int spaces) {
    int depth = spaces / 2;  // Convert spaces to depth units
    
    // Pop containers deeper than current line
    while (state->stack_size > 0 && 
           state->depths[state->stack_size - 1] >= depth) {
        state->stack_size--;
    }
}

// Writing with consistent depth tracking
static void _write_with_depth(FILE *file, const ar_data_t *data, int depth) {
    // Always use 2 * depth spaces for indentation
    for (int i = 0; i < depth * 2; i++) {
        fputc(' ', file);
    }
    // Write content...
}
```

## Testing Strategy
```c
void test_yaml__nested_indentation() {
    // Create deeply nested structure
    ar_data_t *root = ar_data__create_map();
    ar_data_t *level1 = ar_data__create_map();
    ar_data_t *level2 = ar_data__create_list();
    
    ar_data__set_map_data(root, "level1", level1);
    ar_data__set_map_data(level1, "level2", level2);
    ar_data__list_add_last_string(level2, "deep_value");
    
    // Write and verify indentation
    ar_yaml__write_to_file(root, "test.yaml");
    
    // Read file and check indentation
    FILE *f = fopen("test.yaml", "r");
    char line[256];
    int expected_indent = 0;
    while (fgets(line, sizeof(line), f)) {
        int actual_spaces = strspn(line, " ");
        assert(actual_spaces == expected_indent * 2);
        // Update expected based on content...
    }
    fclose(f);
    
    ar_data__destroy(root);
}
```

## Related Patterns
- [Container Stack Parsing Pattern](container-stack-parsing-pattern.md)
- [YAML Serialization Direct I/O Pattern](yaml-serialization-direct-io-pattern.md)
- [Multi-Line Data Persistence Format](multi-line-data-persistence-format.md)