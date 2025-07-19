# Expression Evaluator Accessor Extension

## Learning
Adding new base accessors (like "message") to the expression evaluator requires modifying the memory access evaluation logic while maintaining const-correctness and consistency with existing accessors.

## Importance
The expression evaluator is central to the AgeRun language, allowing access to memory, context, and messages. Understanding how to extend it properly ensures new features integrate seamlessly with the existing expression syntax.

## Example
```c
// From ar_expression_evaluator.c - Adding message accessor
static ar_data_t* _evaluate_memory_access(const ar_expression_evaluator_t *ref_evaluator,
                                         const ar_frame_t *ref_frame,
                                         const ar_expression_ast_t *ref_ast) {
    const char *base = ar_expression_ast__get_base(ref_ast);
    ar_data_t *map = NULL;
    
    if (strcmp(base, "memory") == 0) {
        map = ar_frame__get_memory(ref_frame);
    } else if (strcmp(base, "context") == 0) {
        map = ar_frame__get_context(ref_frame);
    } else if (strcmp(base, "message") == 0) {
        // New accessor - handle const-correctness
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        map = (ar_data_t *)ar_frame__get_message(ref_frame);
        #pragma GCC diagnostic pop
    } else {
        ar_log__error(ref_evaluator->ref_log, 0, 
                     "Unknown base accessor: %s", base);
        return NULL;
    }
    
    // Rest of path traversal logic remains the same
    ar_list_t *ref_path = ar_expression_ast__get_path(ref_ast);
    if (!ref_path || ar_list__count(ref_path) == 0) {
        return map;  // Just the base accessor
    }
    
    // Traverse the path...
}

// Testing the new accessor
void test_expression_evaluator__message_access() {
    // Create test message
    ar_data_t *message = ar_data__create_map();
    ar_data__set_map_string(message, "type", "request");
    ar_data__set_map_integer(message, "id", 42);
    
    // Create frame with message
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    
    // Test base message access
    ar_data_t *result = ar_expression_evaluator__evaluate(evaluator, frame, "message");
    assert(result == message);  // Returns reference
    
    // Test nested access
    result = ar_expression_evaluator__evaluate(evaluator, frame, "message.type");
    assert(strcmp(ar_data__get_string(result), "request") == 0);
    
    ar_frame__destroy(frame);
    ar_data__destroy(message);
}
```

## Generalization
To add a new accessor:
1. **Identify the accessor source**: What data does it access? (frame component, global state, etc.)
2. **Update base accessor logic**: Add to the if-else chain in `_evaluate_memory_access`
3. **Handle const-correctness**: Use pragma directives if needed for const casts
4. **Maintain reference semantics**: Return references, not copies
5. **Add comprehensive tests**: Test base access and nested path access
6. **Update documentation**: Document the new accessor in relevant modules

## Implementation
```c
// Pattern for adding new accessor
else if (strcmp(base, "new_accessor") == 0) {
    // Get data source - often from frame
    const ar_data_t *const_data = get_data_source(ref_frame);
    
    // Handle const if needed
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-qual"
    map = (ar_data_t *)const_data;
    #pragma GCC diagnostic pop
}

// Ensure consistent behavior:
// 1. Returns references (not owned)
// 2. Supports nested access via paths
// 3. Handles NULL gracefully
// 4. Reports errors via log
```

## Related Patterns
- [Expression Ownership Rules](expression-ownership-rules.md)
- [Const Correctness Principle](const-correctness-principle.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [Test Fixture API Adaptation](test-fixture-api-adaptation.md)