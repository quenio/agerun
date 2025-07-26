# Zig Defer for Error Cleanup Pattern

## Problem

In C code, especially in evaluators, error handling requires manual cleanup at every error point, leading to:

1. **Code duplication**: Same cleanup logic repeated at multiple return points
2. **Error-prone maintenance**: Easy to forget cleanup when adding new error paths
3. **Cascading complexity**: Each new resource adds more cleanup code to all error paths

### Example C Pattern

```c
void **items = ar_list__items(ref_arg_asts);
if (!items) return false;

ar_data_t *result1 = evaluate(...);
if (!result1) {
    AR__HEAP__FREE(items);
    return false;
}

ar_data_t *own1 = ar_data__claim_or_copy(result1, owner);
if (!own1) {
    ar_data__destroy_if_owned(result1, owner);
    AR__HEAP__FREE(items);
    return false;
}

ar_data_t *result2 = evaluate(...);
if (!result2) {
    ar_data__destroy(own1);
    AR__HEAP__FREE(items);
    return false;
}
// ... and so on, with growing cleanup chains
```

## Solution: Zig's Defer Mechanism

Zig's `defer` statement executes code when leaving the current scope, regardless of how (return, error, break, etc.). This provides automatic cleanup without manual duplication.

### Zig Pattern with Defer

```zig
const own_items = c.ar_list__items(ref_arg_asts) orelse return false;
defer ar_allocator.free(own_items);

const result1 = evaluate(...) orelse return false;
const own1 = ar_data__claim_or_copy(result1, owner) orelse return false;
defer c.ar_data__destroy(own1);

const result2 = evaluate(...) orelse return false;
const own2 = ar_data__claim_or_copy(result2, owner) orelse return false;
defer c.ar_data__destroy(own2);

// Clean, linear code with automatic cleanup on any return path
```

## Benefits

1. **Eliminates duplication**: Each cleanup is written once, next to the allocation
2. **Prevents bugs**: Can't forget to clean up - it's automatic
3. **Improves readability**: Linear flow without cascading error handling
4. **Scales well**: Adding new resources doesn't complicate existing error paths

## Implementation Strategy

### When to Use This Pattern

Consider Zig migration when a C module has:
- Multiple resources requiring cleanup
- Repeated cleanup code at error points
- Complex error handling cascades
- Growing maintenance burden from error paths

### Migration Approach

1. **Identify cleanup patterns**: Look for repeated `free`/`destroy` calls
2. **Count duplication**: Estimate lines saved by using `defer`
3. **Migrate incrementally**: Start with simpler evaluators
4. **Use ar_allocator**: Follow Zig module conventions
5. **Verify behavior**: Ensure tests pass with zero memory leaks

### Example: ar_exit_instruction_evaluator

The migration of `ar_exit_instruction_evaluator` demonstrates the pattern:

**Before (C)**: Manual cleanup at multiple error points
**After (Zig)**: Single `defer` statement handles all cleanup paths

This eliminated ~100+ lines of duplicated cleanup code across the evaluator.

## Testing Error Cleanup

To verify that defer/errdefer cleanup works correctly, create dedicated error tests:

```c
// Example from ar_instruction_evaluator_error_tests.c
// Override creation functions to simulate failures at specific points
static int fail_at_evaluator = 3;  // Fail at 3rd sub-component

// Track resource lifecycle
static int created = 0;
static int destroyed = 0;

// Mock that can fail on demand
SomeType* some_type__create() {
    if (++current == fail_at_evaluator) return NULL;
    created++;
    return (SomeType*)0x1000;  // Fake pointer
}

// Verify cleanup is called
void some_type__destroy(SomeType* obj) {
    if (obj) destroyed++;
}

// After test: verify created == destroyed
```

This approach verifies that errdefer statements execute in the correct order and all resources are cleaned up.

## Best Practices

1. **Place defer immediately after resource acquisition**:
   ```zig
   const resource = acquire_resource() orelse return error.Failed;
   defer release_resource(resource);
   ```

2. **Use ar_allocator for consistency**:
   ```zig
   const own_data = ar_allocator.create(Type, "description") orelse return null;
   defer ar_allocator.free(own_data);
   ```

3. **Combine with Zig's error unions and inline error handling**:
   ```zig
   // Real example from ar_assignment_instruction_evaluator
   const own_path = c.ar_path__create_variable(ref_path) orelse {
       c.ar_log__error(ref_evaluator.?.ref_log, "Invalid assignment path");
       return false;
   };
   defer c.ar_path__destroy(own_path);
   ```

4. **Order matters - defers execute in reverse order**:
   ```zig
   defer cleanup_second();  // Executes second
   defer cleanup_first();   // Executes first
   ```

5. **Limitations - Manual cleanup still needed when resource is used later**:
   ```zig
   // Can't use defer here - we need 'result' later
   const result = c.ar_expression_evaluator__evaluate(...) orelse {
       return false;
   };
   
   // Must manually cleanup on this error path
   const mut_memory = c.ar_frame__get_memory(ref_frame) orelse {
       c.ar_data__destroy(result);  // Manual cleanup required
       return false;
   };
   ```

## Related Patterns

- [Zig errdefer Value Capture Pattern](zig-errdefer-value-capture-pattern.md) - Understanding how errdefer captures values
- [C to Zig Module Migration](c-to-zig-module-migration.md) - General migration guide
- [Zig Integration Comprehensive](zig-integration-comprehensive.md) - Full integration details
- [Zig Memory Allocation with ar_allocator](zig-memory-allocation-with-ar-allocator.md) - Memory patterns
- [Evaluator Migration Priority Strategy](evaluator-migration-priority-strategy.md) - Complexity-based migration ordering
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md) - Verifying zero leaks after migration

## Conclusion

Migrating evaluators from C to Zig specifically to leverage `defer` for error cleanup is a valid and effective strategy. It solves a real problem (error cleanup duplication) with a language feature designed for exactly this purpose, resulting in cleaner, more maintainable code.