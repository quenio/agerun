# Zig Error Path Testing Pattern

## Learning
Testing error paths in Zig modules requires special techniques since errdefer only triggers on error returns. AgeRun uses C-based function interception to verify that Zig's cleanup mechanisms work correctly.

## Importance
Without testing error paths, memory leaks in errdefer cleanup would go undetected. The standard test suite typically only tests success paths, leaving error handling untested.

## Example
```c
// From ar_instruction_evaluator_error_tests.c
// This test verifies that the Zig implementation properly cleans up when 
// sub-component creation fails at any point

// Global control for simulating failures
static int fail_at_evaluator = -1;
static int current_evaluator = 0;

// Tracking for leak detection
static int expression_created = 0;
static int expression_destroyed = 0;

// Intercept the real function to simulate failures
ar_expression_evaluator_t* ar_expression_evaluator__create(ar_log_t* log) {
    (void)log;
    current_evaluator++;
    if (current_evaluator == fail_at_evaluator) {
        printf("Mock: Failing expression evaluator creation\n");
        return NULL;  // Trigger errdefer in Zig
    }
    expression_created++;
    return (ar_expression_evaluator_t*)0x1000;  // Fake but valid pointer
}

// Track cleanup
void ar_expression_evaluator__destroy(ar_expression_evaluator_t* evaluator) {
    if (evaluator) {
        expression_destroyed++;
        printf("Mock: Destroyed expression evaluator\n");
    }
}

// Test different failure points
void test_cleanup_on_late_failure(void) {
    fail_at_evaluator = 10;  // Fail at the 10th sub-component
    ar_instruction_evaluator_t* eval = ar_instruction_evaluator__create(log);
    assert(eval == NULL);
    
    // Verify all 9 created components were destroyed
    assert(expression_created == expression_destroyed);
    // ... check other components ...
}
```

## Implementation Pattern

1. **Create separate error test file**: Name it `<module>_error_tests.c`

2. **Include necessary headers**:
```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "module_under_test.h"
// Include headers for all dependencies you'll mock
```

3. **Define tracking variables**:
```c
#define DEFINE_INTERCEPTOR(name) \
    static int name##_created = 0; \
    static int name##_destroyed = 0;

DEFINE_INTERCEPTOR(component1)
DEFINE_INTERCEPTOR(component2)
// ... for each component
```

4. **Implement mock functions**:
```c
// Control which function fails
static int fail_at = -1;
static int current = 0;

ComponentType* component__create(/* params */) {
    current++;
    if (current == fail_at) {
        return NULL;  // Simulate failure
    }
    component1_created++;
    return (ComponentType*)0x1000;  // Fake pointer
}

void component__destroy(ComponentType* c) {
    if (c) component1_destroyed++;
}
```

5. **Write test functions**:
```c
static void test_module__cleanup_on_early_failure(void) {
    reset_counters();
    fail_at = 1;  // Fail at first component
    
    ModuleType* m = module__create();
    assert(m == NULL);
    
    // No components should have been created
    assert(component1_created == 0);
}

static void test_module__cleanup_on_late_failure(void) {
    reset_counters();
    fail_at = 3;  // Fail at third component
    
    ModuleType* m = module__create();
    assert(m == NULL);
    
    // First two components should be created and destroyed
    assert(component1_created == 1);
    assert(component1_destroyed == 1);
    assert(component2_created == 1);
    assert(component2_destroyed == 1);
}
```

## Key Techniques

### Function Interception
The test overrides the actual functions by:
1. Defining functions with the same signature
2. Linking the test before the library containing the real functions
3. The linker uses the first definition it finds

### Fake Pointers
Return non-NULL pointers like `(Type*)0x1000` to simulate successful allocation without actually allocating memory.

### ISO C Compliance
For dlsym() usage (if needed), use a union to avoid warnings:
```c
union { void* obj; func_ptr_type func; } converter;
converter.obj = dlsym(RTLD_NEXT, "function_name");
real_function = converter.func;
```

## Benefits

1. **Catches memory leaks**: Verifies all errdefer statements execute
2. **Tests error propagation**: Ensures errors bubble up correctly
3. **Validates cleanup order**: Confirms resources are freed in reverse order
4. **No code changes needed**: Tests the actual production code
5. **Comprehensive coverage**: Can test every possible failure point

## Limitations

1. **Link order matters**: Test must be linked before the library
2. **Platform specific**: Function interception is platform-dependent
3. **Can't test static functions**: Only exported functions can be intercepted

## Best Practices

1. **Test all failure points**: Create a test for each component that can fail
2. **Verify cleanup order**: Print messages to confirm reverse-order cleanup
3. **Check all resources**: Track every allocation and deallocation
4. **Use meaningful names**: `test_module__cleanup_on_<specific>_failure`
5. **Reset state between tests**: Clear all counters before each test

## Related Patterns
- [Zig errdefer Value Capture Pattern](zig-errdefer-value-capture-pattern.md) - Understanding errdefer mechanics
- [Zig Defer for Error Cleanup](zig-defer-error-cleanup-pattern.md) - Using defer for cleanup
- [C to Zig Module Migration](c-to-zig-module-migration.md) - Migration strategies