# Zig errdefer Value Capture Pattern

## Learning
`errdefer` captures the **value** of variables at the time it's registered, not a reference to the variable. This enables precise cleanup of resources based on initialization progress. **CRITICAL**: `errdefer` only triggers on error returns, NOT on `orelse return null`.

## Importance
Understanding value capture is crucial for correct error handling in Zig. Misunderstanding this can lead to attempting to destroy null pointers or missing cleanup entirely. More critically, using `errdefer` with nullable returns (`orelse return null`) will cause memory leaks because the cleanup never executes.

## Example of the Problem
```zig
// INCORRECT - This causes memory leaks!
const own_evaluator = ar_allocator.create(ar_instruction_evaluator_t, "instruction_evaluator") orelse return null;
errdefer ar_allocator.free(own_evaluator);  // WARNING: Never executes on null return!

own_evaluator.ref_log = ref_log;

// Create the expression evaluator
own_evaluator.own_expr_evaluator = c.ar_expression_evaluator__create(ref_log) orelse return null;
errdefer c.ar_expression_evaluator__destroy(own_evaluator.own_expr_evaluator);  // WARNING: Never executes!

// If we fail here with 'orelse return null', NO cleanup happens!
own_evaluator.own_assignment_evaluator = c.ar_assignment_instruction_evaluator__create(...) orelse return null;
// The errdefers DO NOT execute because we're returning null, not an error!
```

## How Value Capture Works

### The Capture Moment
Each `errdefer` captures the value **at the exact moment the errdefer statement executes**:

```zig
// Step-by-step value capture
own_evaluator.own_expr_evaluator = create() orelse return null;  // Returns 0xAAA
errdefer destroy(own_evaluator.own_expr_evaluator);  // Captures 0xAAA (not the field reference!)

own_evaluator.own_send_evaluator = create() orelse return null;  // Returns 0xBBB  
errdefer destroy(own_evaluator.own_send_evaluator);  // Captures 0xBBB

own_evaluator.own_build_evaluator = create() orelse return null;  // Returns null - triggers cleanup!
// Errdefers execute with their captured values:
// - destroy(0xBBB)  ← Destroys send evaluator
// - destroy(0xAAA)  ← Destroys expression evaluator
```

### Not Dynamic References
**Critical**: `errdefer` does NOT capture a reference that updates dynamically:

```zig
// WRONG - captures current value (likely null or uninitialized)
errdefer c.ar_expression_evaluator__destroy(own_evaluator.own_expr_evaluator);
own_evaluator.own_expr_evaluator = c.ar_expression_evaluator__create(ref_log) orelse return null;

// CORRECT - captures the successfully created pointer
own_evaluator.own_expr_evaluator = c.ar_expression_evaluator__create(ref_log) orelse return null;
errdefer c.ar_expression_evaluator__destroy(own_evaluator.own_expr_evaluator);
```

## Common Patterns

### Progressive Resource Acquisition
```zig
// Each successful allocation gets its own errdefer
const resource1 = allocate1() orelse return error.Failed;
errdefer free1(resource1);  // Captures resource1's pointer

const resource2 = allocate2() orelse return error.Failed;
errdefer free2(resource2);  // Captures resource2's pointer

const resource3 = allocate3() orelse return error.Failed;
errdefer free3(resource3);  // Captures resource3's pointer

// If allocate4 fails, cleanup happens in reverse order with captured values
const resource4 = allocate4() orelse return error.Failed;
```

### Struct Field Initialization
```zig
const own_struct = allocator.create(MyStruct) orelse return null;
errdefer allocator.free(own_struct);

// Initialize fields one by one with errdefer after each success
own_struct.field1 = createField1() orelse return null;
errdefer destroyField1(own_struct.field1);  // Captures the created value

own_struct.field2 = createField2() orelse return null;
errdefer destroyField2(own_struct.field2);  // Captures this specific value
```

## Why This Design

1. **Precision**: Each errdefer knows exactly what to clean up
2. **Safety**: Can't accidentally destroy uninitialized memory
3. **Order**: Cleanup happens in reverse order of acquisition
4. **Simplicity**: No need for complex state tracking

## Correct Pattern for Nullable Returns

When functions return nullable types (like AgeRun's create functions), use a private error-returning implementation:

```zig
// Private implementation that uses error unions and errdefer
fn _create(ref_log: ?*c.ar_log_t) !*ar_instruction_evaluator_t {
    if (ref_log == null) return error.NullParameter;
    
    const own_evaluator = ar_allocator.create(ar_instruction_evaluator_t, "instruction_evaluator") orelse 
        return error.OutOfMemory;
    errdefer ar_allocator.free(own_evaluator);  // NOW this works!
    
    own_evaluator.ref_log = ref_log;
    
    // Create the expression evaluator
    own_evaluator.own_expr_evaluator = c.ar_expression_evaluator__create(ref_log) orelse 
        return error.SubEvaluatorCreationFailed;
    errdefer c.ar_expression_evaluator__destroy(own_evaluator.own_expr_evaluator);  // Works!
    
    // Create assignment evaluator
    own_evaluator.own_assignment_evaluator = c.ar_assignment_instruction_evaluator__create(
        ref_log,
        own_evaluator.own_expr_evaluator
    ) orelse return error.SubEvaluatorCreationFailed;
    errdefer c.ar_assignment_instruction_evaluator__destroy(own_evaluator.own_assignment_evaluator);
    
    // Continue pattern... all errdefers work correctly
    return own_evaluator;
}

// Public ABI-compatible function
export fn ar_instruction_evaluator__create(ref_log: ?*c.ar_log_t) ?*ar_instruction_evaluator_t {
    return _create(ref_log) catch null;
}
```

This pattern:
- Keeps the clean errdefer syntax internally
- Maintains ABI compatibility 
- Avoids verbose manual cleanup
- Properly triggers errdefer on errors

## Common Mistakes

### Using errdefer with Nullable Returns
```zig
// DON'T DO THIS - errdefer doesn't work with 'orelse return null'
const resource = allocate() orelse return null;
errdefer free(resource);  // NEVER EXECUTES!
```

### Capturing Too Early
```zig
// DON'T DO THIS
var ptr: ?*MyType = null;
errdefer if (ptr) |p| destroy(p);  // Captures null!
ptr = create();  // errdefer still has null
```

### Assuming Reference Semantics
```zig
// WRONG MENTAL MODEL
var changing_value: i32 = 5;
errdefer print("{}", .{changing_value});  // Prints 5, not 10!
changing_value = 10;
```

## Best Practices

1. **Place errdefer immediately after successful resource acquisition**
2. **One errdefer per resource** for clarity
3. **Trust the value capture** - no need for null checks if placed correctly
4. **Use for complex initialization** where manual cleanup would be error-prone
5. **Test error paths** - Create dedicated error tests to verify cleanup (see Testing section)

## Testing Error Paths

Since errdefer only triggers on error returns, it's crucial to test failure paths. AgeRun uses function interception in C to verify cleanup:

```c
// From ar_instruction_evaluator_error_tests.c
// Mock functions track creation/destruction
static int expression_created = 0;
static int expression_destroyed = 0;

ar_expression_evaluator_t* ar_expression_evaluator__create(ar_log_t* log) {
    (void)log;
    current_evaluator++;
    if (current_evaluator == fail_at_evaluator) {
        printf("Mock: Failing expression evaluator creation\n");
        return NULL;  // Simulate failure
    }
    expression_created++;
    return (ar_expression_evaluator_t*)0x1000;  // Fake pointer
}

void ar_expression_evaluator__destroy(ar_expression_evaluator_t* evaluator) {
    if (evaluator) {
        expression_destroyed++;
        printf("Mock: Destroyed expression evaluator\n");
    }
}
```

This approach:
1. **Intercepts real functions** - Overrides module functions to simulate failures
2. **Tracks allocations** - Counts creates and destroys to detect leaks
3. **Verifies cleanup order** - Ensures errdefer executes in reverse order
4. **Tests all failure points** - Can fail at any sub-component creation

The test file naming convention is `*_error_tests.c` to distinguish from regular tests.

## Relationship to Other Patterns

This value capture behavior is why the facade pattern in `ar_instruction_evaluator` works so elegantly - each evaluator's cleanup is registered with its exact pointer value immediately after successful creation.

## Official Documentation
For the official Zig documentation on errdefer, see: [https://ziglang.org/documentation/0.14.1/#errdefer](https://ziglang.org/documentation/0.14.1/#errdefer)

## Related Patterns
- [Zig Defer for Error Cleanup](zig-defer-error-cleanup-pattern.md) - General defer patterns
- [C to Zig Module Migration](c-to-zig-module-migration.md) - Migration strategies
- [Zig Memory Allocation with ar_allocator](zig-memory-allocation-with-ar-allocator.md) - Memory patterns