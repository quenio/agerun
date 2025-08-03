# Memory Debugging Comprehensive Guide

## Learning
How to systematically debug memory leaks in AgeRun by tracing allocation sources, verifying ownership semantics, and fixing variable naming to reflect actual ownership.

## Importance
Memory leaks are critical bugs that can degrade performance and cause system instability. Understanding the debugging workflow helps identify and fix leaks quickly.

## Example
```c
// SYMPTOM: Memory leak detected in test_instruction_parser
// 1. Isolate the specific test causing the leak
// 2. Check memory report: bin/memory_report_instruction_parser_tests.log
//    Shows: "ar_data_t (list) allocated at instruction_ast.c:142"

// 3. Examine the leaking function:
ar_data_t* ar_instruction_ast__get_function_args(ar_instruction_ast_t *ast) {
    return ar_data__create_list();  // Creates NEW list (ownership transfer)
}

// 4. Find usage in tests - variable naming reveals the bug:
ar_data_t *ref_args = ar_instruction_ast__get_function_args(ast);  // WRONG: ref_ implies borrowed
// ... no ar_data__destroy(ref_args) call found

// 5. Fix by updating variable name and adding cleanup:
ar_data_t *own_args = ar_instruction_ast__get_function_args(ast);  // Correct prefix
// ... use args ...
ar_data__destroy(own_args);  // Add cleanup

// 6. Common patterns that cause leaks:
// - Removal functions: ar_data__list_remove_first() returns owned value
// - Map iteration: ar_data__get_map_keys() creates new list
// - String operations: "Hello" + " World" creates new string
// - Buffer overflows: escape sequences need 2 bytes, not 1
// - Don't trust function names - check ownership docs
```

## Generalization
When leak detected → Check memory report → Trace allocation source → Verify ownership semantics → Fix variable naming → Add proper cleanup

## Implementation
1. Run test with `make test_name 2>&1` (capture stderr for warnings!)
2. Check `bin/memory_report_<test_name>.log` for leak details
3. Examine the allocating function to understand ownership
4. Review all call sites for proper variable naming (own_/mut_/ref_)
5. Add missing `ar_data__destroy()` calls for owned data
6. Re-run test to verify leak is fixed

**Use-After-Free Prevention**: Watch error paths + ownership transfer interactions. Run ASan always when both present.

## Wake Message Memory Leaks
A common pattern discovered: agents automatically send themselves wake messages on creation, which must be processed to avoid leaks:
```c
// Common leak pattern in tests
int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, "method", "1.0.0", NULL);
// Wake message sits in queue - MEMORY LEAK!

// Fix: Process the wake message
int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, "method", "1.0.0", NULL);
ar_system__process_next_message_with_instance(own_system);  // Processes and frees wake message
```


**Note**: Examples assume `own_system`, `mut_agency`, and other instance variables are available. In practice, these would be created via fixtures or passed as parameters.
This pattern caused identical leaks (2 allocations, ~41-45 bytes) across 8+ test files.

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Temporary Resource Ownership Pattern](temporary-resource-ownership-pattern.md) - Common cause of context leaks
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
- [Agent Wake Message Processing](agent-wake-message-processing.md)
- [Stderr Redirection for Debugging](stderr-redirection-debugging.md) - Leak warnings go to stderr