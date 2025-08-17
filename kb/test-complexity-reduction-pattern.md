# Test Complexity Reduction Pattern

## Learning
When debugging test failures, especially in complex scenarios like nested data structures, simplify the test case to the minimal example that reproduces the issue. This isolates the problem and makes debugging more tractable.

## Importance
Complex tests with multiple nested structures and many fields can obscure the root cause of failures. By systematically reducing complexity while preserving the failure condition, you can identify the exact parsing or serialization issue without the noise of unrelated data.

## Example
**Original complex test that failed:**
```c
void test_yaml__full_agent_structure() {
    // Complex structure with many fields
    ar_data_t *own_data = ar_data__create_map();
    ar_data__set_map_string(own_data, "agency_version", "1.0.0");
    
    ar_data_t *own_agents = ar_data__create_list();
    ar_data_t *own_agent1 = ar_data__create_map();
    ar_data__set_map_integer(own_agent1, "id", 1);
    ar_data__set_map_string(own_agent1, "name", "manager");
    ar_data__set_map_string(own_agent1, "method", "manager-1.0.0");
    
    ar_data_t *own_memory1 = ar_data__create_map();
    ar_data__set_map_string(own_memory1, "role", "coordinator");
    ar_data__set_map_integer(own_memory1, "tasks_completed", 42);
    ar_data__set_map_data(own_agent1, "memory", own_memory1);
    
    ar_data__list_add_last_data(own_agents, own_agent1);
    // ... more agents ...
    ar_data__set_map_data(own_data, "agents", own_agents);
    
    // Test fails here - but why?
    ar_yaml__write_to_file(own_data, "test.yaml");
    ar_data_t *own_loaded = ar_yaml__read_from_file("test.yaml");
    // Complex assertion fails...
}
```

**Simplified to isolate the issue:**
```c
void test_yaml__minimal_nested_map() {
    // Reduced to minimal failing case
    ar_data_t *own_data = ar_data__create_map();
    ar_data_t *own_list = ar_data__create_list();
    ar_data_t *own_map = ar_data__create_map();
    
    // Just one nested field
    ar_data__set_map_string(own_map, "key", "value");
    ar_data__list_add_last_data(own_list, own_map);
    ar_data__set_map_data(own_data, "items", own_list);
    
    // Now the exact failure point is clear
    ar_yaml__write_to_file(own_data, "test.yaml");
    
    // Can inspect the exact YAML output
    // items:
    //   - key: value  <- Issue with indentation visible!
}
```

## Generalization
Test simplification process:
1. **Start with failing test**: Identify complex test that fails
2. **Remove non-essential fields**: Strip out fields one by one
3. **Reduce nesting levels**: Simplify to minimal nesting that still fails
4. **Use simple values**: Replace complex data with "test", 0, etc.
5. **Single container**: Reduce lists/maps to single item
6. **Verify failure persists**: Ensure simplified test still reproduces issue
7. **Debug minimal case**: Much easier to trace through simple structure
8. **Apply fix**: Fix the root cause in the minimal case
9. **Restore complexity**: Gradually add back complexity to verify fix

## Implementation
```c
// Debug helper for complex test reduction
void debug_yaml_structure(const char *description, ar_data_t *data) {
    printf("\n=== %s ===\n", description);
    ar_yaml__write_to_file(data, "debug.yaml");
    
    // Print raw YAML for inspection
    FILE *f = fopen("debug.yaml", "r");
    char line[256];
    int line_no = 1;
    while (fgets(line, sizeof(line), f)) {
        printf("%3d: %s", line_no++, line);
    }
    fclose(f);
    
    // Try to read it back
    ar_data_t *loaded = ar_yaml__read_from_file("debug.yaml");
    if (loaded) {
        printf("Read successful\n");
        ar_data__destroy(loaded);
    } else {
        printf("Read FAILED\n");
    }
    
    unlink("debug.yaml");
}

// Use in test development
void test_yaml__debug_complex_structure() {
    ar_data_t *own_complex = create_complex_structure();
    debug_yaml_structure("Original complex", own_complex);
    
    // Simplify step by step
    ar_data_t *own_simpler = create_simpler_structure();
    debug_yaml_structure("Simplified", own_simpler);
    
    ar_data_t *own_minimal = create_minimal_structure();
    debug_yaml_structure("Minimal", own_minimal);
    
    // Clean up
    ar_data__destroy(own_complex);
    ar_data__destroy(own_simpler);
    ar_data__destroy(own_minimal);
}
```

## Common Simplification Patterns
```c
// Complex -> Simple transformations

// Multiple items -> Single item
ar_data__list_add_last_string(list, "item1");
ar_data__list_add_last_string(list, "item2");  // Remove
ar_data__list_add_last_string(list, "item3");  // Remove

// Many fields -> One field
ar_data__set_map_string(map, "field1", "value1");
ar_data__set_map_string(map, "field2", "value2");  // Remove
ar_data__set_map_integer(map, "field3", 123);      // Remove

// Deep nesting -> Shallow
ar_data_t *level1 = ar_data__create_map();
ar_data_t *level2 = ar_data__create_map();
ar_data_t *level3 = ar_data__create_map();  // Remove this level

// Complex values -> Simple values
ar_data__set_map_string(map, "description", 
    "A very long string with special chars !@#$");
// Simplify to:
ar_data__set_map_string(map, "key", "value");
```

## Related Patterns
- [Test Isolation Through Commenting](test-isolation-through-commenting.md)
- [Debug Program Recompilation Pattern](debug-program-recompilation-pattern.md)
- [Container Stack Parsing Pattern](container-stack-parsing-pattern.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)