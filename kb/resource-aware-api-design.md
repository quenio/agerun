# Resource-Aware API Design

## Learning
API design decisions that seem convenient can hide significant resource costs. User feedback about memory usage, performance, or resource consumption should trigger immediate reconsideration of the entire API design, not just optimization of the existing approach.

## Importance
Resource-inefficient APIs compound their cost with every use. A design that duplicates memory for convenience will cause that duplication at every call site. Early feedback about resource usage can prevent systemic inefficiency from being built into the architecture.

## Example
Original design with hidden memory duplication:
```c
// BAD: Convenient API that hides memory cost
// EXAMPLE: This function was removed after user feedback about memory usage
char* yaml_to_string_bad(const ar_data_t *ref_data) {
    // Would build entire string in memory
    return strdup("yaml content");
}

bool yaml_write_inefficient(const ar_data_t *ref_data, const char *filename) {
    char *yaml_string = yaml_to_string_bad(ref_data);  // Memory duplication!
    FILE *file = fopen(filename, "w");
    fprintf(file, "%s", yaml_string);
    fclose(file);
    free(yaml_string);  // Extra allocation/deallocation
    return true;
}

// User feedback: "We do not want to convert the ar_data_t struct into/from string 
// because that will increment the memory requirements of this system."

// Complete redesign based on resource awareness:
bool ar_yaml__write_to_file(const ar_data_t *ref_data, const char *ref_filename) {
    FILE *file = fopen(ref_filename, "w");
    if (!file) return false;
    
    // Direct streaming - no intermediate allocations
    _write_yaml_to_file(file, ref_data, 0, false);
    
    fclose(file);
    return true;
}
```

## Generalization
Resource-aware API design principles:
1. **Question convenience functions**: APIs that return allocated strings often hide memory costs
2. **Prefer streaming over buffering**: Process data as it flows rather than accumulating
3. **Make resource costs visible**: If allocation is necessary, make it explicit in the API
4. **Listen to resource concerns immediately**: Don't defer optimization - redesign early
5. **Consider the multiplication effect**: APIs are called many times - costs compound

Red flags in API design:
- Functions returning allocated strings for serialization
- Intermediate representations that duplicate data
- "Convenience" functions that hide allocations
- APIs that require full materialization of large structures

## Implementation
When receiving feedback about resource usage:
```bash
# 1. Stop implementation immediately
# 2. Analyze the resource cost multiplication
#    "How many times will this API be called?"
#    "What's the total memory impact?"

# 3. Consider streaming alternatives
#    Can we process incrementally?
#    Can we avoid intermediate representations?

# 4. Redesign the API completely if needed
#    Better to redesign early than optimize later

# 5. Document the resource-aware design choice
#    Explain why the API works this way
```

Example refactoring pattern:
```c
// From buffering to streaming
// Before: Build complete string
char* serialize_to_string(const ar_data_t *data);  // EXAMPLE: Hypothetical function

// After: Stream directly to destination  
void serialize_to_file(FILE *file, const ar_data_t *data);
void serialize_to_buffer(char *buffer, size_t size, const ar_data_t *data);
```

## Related Patterns
- [Memory-Efficient Streaming Patterns](memory-efficient-streaming-patterns.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Architectural Review in Feedback](architectural-review-in-feedback.md)
- [API Design for Change](design-for-change-principle.md)