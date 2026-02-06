# DLSym Malloc Retry Logic Pattern

## Learning
When testing malloc failures using dlsym interception, `ar_heap__malloc` and related functions (`ar_heap__strdup`, `ar_heap__calloc`, `ar_heap__realloc`) implement retry logic that calls `malloc` again after the initial failure. To truly test error handling paths, dlsym tests must fail consecutive mallocs, not just a single allocation.

## Importance
Without handling retry logic, dlsym tests appear to pass but don't actually verify error handling. The retry succeeds, the function returns successfully, and tests incorrectly report that error handling works. This creates false confidence in error handling code and allows bugs to slip through.

## Example

**Problem: Failing single malloc doesn't test error handling**

```c
// BAD: Only fails one malloc - retry succeeds
void* malloc(size_t size) {
    static int current_malloc = 0;
    current_malloc++;
    
    if (current_malloc == 5) {
        return NULL;  // Fail malloc #5
    }
    
    return real_malloc(size);
}

// Test result: PASSES (incorrectly)
// Why: ar_heap__malloc retries at malloc #6, succeeds, function returns non-NULL
```

**Solution: Fail consecutive mallocs to defeat retry logic**

```c
// GOOD: Fails consecutive mallocs - retry also fails
static int fail_at_malloc = -1;
static int fail_count = 1;  // Number of consecutive mallocs to fail
static int current_malloc = 0;
static int consecutive_failures = 0;

void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;

    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }

    current_malloc++;
    
    // Check if we should fail this malloc
    if (fail_at_malloc > 0 && current_malloc >= fail_at_malloc && consecutive_failures < fail_count) {
        printf("  Mock: Failing malloc #%d (consecutive failure %d/%d)\n", 
               current_malloc, consecutive_failures + 1, fail_count);
        consecutive_failures++;
        return NULL;  // Simulate malloc failure
    }

    return real_malloc(size);
}

// Reset counters before each test
static void reset_counters(void) {
    fail_at_malloc = -1;
    fail_count = 1;
    current_malloc = 0;
    consecutive_failures = 0;
}

// Test: Fail both initial malloc and retry
static void test_file_delegate__create_handles_malloc_failure_delegate(void) {
    reset_counters();
    fail_at_malloc = 5;  // Start failing at malloc #5 (delegate struct)
    fail_count = 2;      // Fail 2 consecutive mallocs (#5 and #6) to defeat retry logic

    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Now correctly tests error handling - both #5 and #6 fail
    AR_ASSERT(own_delegate == NULL, "Should handle delegate malloc failure");

    ar_log__destroy(ref_log);
}
```

## Generalization

**Retry Logic in ar_heap Functions**:

All `ar_heap__*` allocation functions implement retry logic:
- `ar_heap__malloc`: Retries small allocations (<1024 bytes) with 75% criticality
- `ar_heap__strdup`: Retries with 85% criticality
- `ar_heap__calloc`: Retries with 80% criticality  
- `ar_heap__realloc`: Retries with 95% criticality

**Pattern for dlsym tests**:

1. **Add consecutive failure tracking**:
   ```c
   static int fail_count = 1;  // Number of consecutive mallocs to fail
   static int consecutive_failures = 0;
   ```

2. **Fail multiple consecutive mallocs**:
   ```c
   if (current_malloc >= fail_at_malloc && consecutive_failures < fail_count) {
       consecutive_failures++;
       return NULL;
   }
   ```

3. **Reset counters between tests**:
   ```c
   static void reset_counters(void) {
       fail_at_malloc = -1;
       fail_count = 1;
       current_malloc = 0;
       consecutive_failures = 0;
   }
   ```

4. **Set fail_count to 2** (or more if multiple retries possible):
   ```c
   fail_at_malloc = 5;  // Target allocation
   fail_count = 2;      // Fail initial + retry
   ```

**Identifying Correct Malloc Numbers**:

Heap tracking allocations shift expected malloc numbers:
- Use `printf` debugging to trace actual allocation sequence
- Don't assume malloc numbers - verify with output
- Heap tracking creates allocations before the requested memory

**Example debugging output**:
```
Mock: malloc #1 called (size=16)    // log
Mock: malloc #2 called (size=56)   // heap tracking
Mock: malloc #3 called (size=24)   // other allocation
Mock: malloc #4 called (size=56)  // heap tracking
Mock: malloc #5 called (size=24)   // delegate struct (target)
Mock: malloc #6 called (size=24)   // retry of #5
```

## Implementation

**Complete dlsym malloc wrapper with retry handling**:

```c
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

// Control variables for malloc failure injection
static int fail_at_malloc = -1;
static int fail_count = 1;  // Number of consecutive mallocs to fail
static int current_malloc = 0;
static int consecutive_failures = 0;

// dlsym malloc wrapper
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;

    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }

    current_malloc++;
    printf("  Mock: malloc #%d called (size=%zu)\n", current_malloc, size);
    
    // Check if we should fail this malloc
    if (fail_at_malloc > 0 && current_malloc >= fail_at_malloc && consecutive_failures < fail_count) {
        printf("  Mock: Failing malloc #%d (consecutive failure %d/%d)\n", 
               current_malloc, consecutive_failures + 1, fail_count);
        consecutive_failures++;
        return NULL;  // Simulate malloc failure
    }

    return real_malloc(size);
}

// Reset counters before each test
static void reset_counters(void) {
    fail_at_malloc = -1;
    fail_count = 1;
    current_malloc = 0;
    consecutive_failures = 0;
}
```

**Test pattern**:

```c
// Test malloc failure at delegate struct allocation
static void test_file_delegate__create_handles_malloc_failure_delegate(void) {
    // ar_heap__malloc retries on failure, so we need to fail both the initial malloc
    // and the retry. The delegate struct is 24 bytes.
    // From debugging: malloc #5 is 24 bytes and happens after #3/#4 failures
    // We need to fail both #5 and #6 (the retry) to truly fail the delegate struct allocation
    
    reset_counters();
    fail_at_malloc = 5;  // Start failing at malloc #5 (delegate struct)
    fail_count = 2;      // Fail 2 consecutive mallocs (#5 and #6) to defeat retry logic

    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Then should return NULL and not crash
    AR_ASSERT(own_delegate == NULL, "Should handle delegate malloc failure");

    ar_log__destroy(ref_log);
}
```

## Common Mistakes

1. **Failing only one malloc**: Retry succeeds, test passes incorrectly
2. **Assuming malloc numbers**: Heap tracking shifts numbers - must debug
3. **Not resetting counters**: Previous test state affects current test
4. **Wrong fail_count**: Using 1 instead of 2 (or more for multiple retries)

## Verification

**Verify retry logic is defeated**:
```bash
# Run test and check output shows both failures
make ar_file_delegate_dlsym_tests 2>&1 | grep "Failing malloc"
# Should show:
#   Mock: Failing malloc #5 (consecutive failure 1/2)
#   Mock: Failing malloc #6 (consecutive failure 2/2)
```

**Verify error handling works**:
```bash
# Test should return NULL (not crash or succeed)
# Function should handle failure gracefully
make ar_file_delegate_dlsym_tests 2>&1 | grep "PASS"
```

## Related Patterns
- [DLSym Test Interception Technique](dlsym-test-interception-technique.md) - Base technique for function interception
- [Whitelist Success Message Management](whitelist-success-message-management.md) - Whitelisting intentional error messages from dlsym tests
- [Sanitizer Test Exclusion Pattern](sanitizer-test-exclusion-pattern.md) - Excluding dlsym tests from sanitizer builds
- [Test Effectiveness Verification](test-effectiveness-verification.md) - Verifying tests actually catch failures

