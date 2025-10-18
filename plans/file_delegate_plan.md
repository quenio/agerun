# Plan: FileDelegate Implementation

## Overview
Implement a FileDelegate module that safely handles file system operations with security policies and resource limits, following the delegate pattern established in the delegation architecture.

**Pattern**: Similar to ar_agent/ar_agency message passing
**Modules**: ar_file_delegate.h/c (new), ar_delegate.h (interface)
**Status**: DRAFT - PENDING REVIEW

## Objective
Create a delegate type for file operations with path validation, size limits, and secure file I/O using the ar_io module.

## Context
- Existing types: `ar_delegation_t`, `ar_delegate_t` (from delegation system)
- New functions: `ar_file_delegate__create()`, `ar_file_delegate__read()`, `ar_file_delegate__write()`
- Pattern reference: Similar to agent message handling but for file operations
- Security focus: Path validation, size limits, ownership management

## Success Criteria
- All tests pass
- Zero memory leaks
- BDD test structure throughout
- One assertion per iteration
- GREEN minimalism followed
- Path security enforced
- Size limits respected

## Plan Status
- Total iterations: 15
- Cycles: 5 (TDD Cycles 8-12)
- Review status: 0% (all PENDING REVIEW)

---

### TDD Cycle 8: Basic Structure

#### Iteration 8.1: ar_file_delegate__create() returns non-NULL - PENDING REVIEW

**Objective**: Verify FileDelegate creation returns a valid instance

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can actually catch creation failures
- **Method**: Apply temporary corruption, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what code needs to be written
- **Method**: Observe what failing test expects
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__create_returns_non_null(void) {
    // Given a log instance for the delegate
    ar_log_t *ref_log = ar_log__create();

    // When creating a FileDelegate
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // Then it should return non-NULL
    AR_ASSERT(own_delegate != NULL, "FileDelegate should be created");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Initial stub returns NULL
```c
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    return NULL;  // TEMPORARY: Always fail to prove test catches failures
}
```
Expected RED: "Test FAILS at AR_ASSERT with 'FileDelegate should be created'"
Verify: `make ar_file_delegate_tests` → assertion fails

**Evidence of Goal 1 completion**: Test output showing FAILURE

---

**GREEN Phase:**

**GREEN Phase Goal: Make Test Pass**

For **NEW implementation iterations**:
- **Goal 2** needs satisfaction (write minimal implementation)
- Implement ONLY what's needed to pass this specific assertion

**This iteration**: NEW - Need to create and return a non-NULL delegate

```c
typedef struct ar_file_delegate_s {
    ar_log_t *ref_log;
    char *own_allowed_path;
} ar_file_delegate_t;

ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    // Minimal: Just allocate, store references, and return
    ar_file_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t));
    own_delegate->ref_log = ref_log;
    own_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path);
    return own_delegate;  // Non-NULL - passes the assertion
}
```

**Note**: Error handling (NULL checks, allocation failures) will be added in later iterations when tested.

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 8.1.1: ar_file_delegate__create() handles NULL log parameter - PENDING REVIEW

**Objective**: Verify create() returns NULL when log parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__create_handles_null_log(void) {
    // Given a NULL log parameter
    ar_log_t *ref_log = NULL;

    // When creating a FileDelegate with NULL log
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Then it should return NULL
    AR_ASSERT(own_delegate == NULL, "Should reject NULL log");  // ← FAILS (no NULL check)

    // Cleanup (none needed - delegate should be NULL)
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation (from 8.1) doesn't check for NULL, so test will fail.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL log validation

```c
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    if (!ref_log) return NULL;  // Add NULL check for log

    // Rest of implementation from 8.1
    ar_file_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t));
    own_delegate->ref_log = ref_log;
    own_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path);
    return own_delegate;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 8.1.2: ar_file_delegate__create() handles NULL path parameter - PENDING REVIEW

**Objective**: Verify create() returns NULL when path parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__create_handles_null_path(void) {
    // Given a NULL path parameter
    ar_log_t *ref_log = ar_log__create();

    // When creating a FileDelegate with NULL path
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, NULL);

    // Then it should return NULL
    AR_ASSERT(own_delegate == NULL, "Should reject NULL path");  // ← FAILS (no NULL check)

    // Cleanup
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation (from 8.1.1) doesn't check path for NULL, so test will fail.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL path validation

```c
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    if (!ref_log) return NULL;
    if (!ref_allowed_path) return NULL;  // Add NULL check for path

    ar_file_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t));
    own_delegate->ref_log = ref_log;
    own_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path);
    return own_delegate;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 8.1.3: ar_file_delegate__create() handles malloc failure - PENDING REVIEW

**Objective**: Verify create() returns NULL when malloc fails and cleans up properly

**NOTE**: This test uses dlsym interception technique (see [DLSym Test Interception Technique](../kb/dlsym-test-interception-technique.md))

**Test File**: `modules/ar_file_delegate_dlsym_tests.c` (excluded from sanitizer builds)

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch unhandled malloc failure
- **Method**: Use dlsym malloc wrapper with failure control, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine malloc failure handling
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test with dlsym Interception**

```c
// Control variables for malloc failure injection
static int fail_at_malloc = -1;
static int current_malloc = 0;

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
    if (current_malloc == fail_at_malloc) {
        printf("  Mock: Failing malloc #%d\n", current_malloc);
        return NULL;
    }

    return real_malloc(size);
}

// Reset counters before each test
static void reset_counters(void) {
    fail_at_malloc = -1;
    current_malloc = 0;
}

// Test malloc failure at first allocation (delegate struct)
static void test_file_delegate__create_handles_malloc_failure_delegate(void) {
    reset_counters();
    fail_at_malloc = 1;  // Fail first malloc (delegate struct)

    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Then should return NULL and not crash
    AR_ASSERT(own_delegate == NULL, "Should handle delegate malloc failure");  // ← FAILS (no NULL check)

    ar_log__destroy(ref_log);
}

// Test malloc failure at second allocation (strdup path)
static void test_file_delegate__create_handles_malloc_failure_strdup(void) {
    reset_counters();
    fail_at_malloc = 2;  // Fail second malloc (strdup)

    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Then should return NULL and clean up delegate
    AR_ASSERT(own_delegate == NULL, "Should handle strdup failure");  // ← FAILS (no cleanup)

    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Skip NULL checks to cause failures

```c
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    if (!ref_log) return NULL;
    if (!ref_allowed_path) return NULL;

    ar_file_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t));
    // TEMPORARY: Skip NULL check - will crash or leak
    // if (!own_delegate) return NULL;

    own_delegate->ref_log = ref_log;
    own_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path);
    // TEMPORARY: Skip NULL check and cleanup - will leak
    // if (!own_delegate->own_allowed_path) {
    //     AR__HEAP__FREE(own_delegate);
    //     return NULL;
    // }

    return own_delegate;
}
```

Expected RED:
- Test 1 crashes (NULL dereference)
- Test 2 leaks memory (delegate not freed)

Verify: `make ar_file_delegate_dlsym_tests` → assertions fail or crash

---

**GREEN Phase:**

**This iteration**: NEW - Add malloc/strdup failure checks with cleanup

```c
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    if (!ref_log) return NULL;
    if (!ref_allowed_path) return NULL;

    ar_file_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t));
    if (!own_delegate) return NULL;  // Handle malloc failure

    own_delegate->ref_log = ref_log;
    own_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path);
    if (!own_delegate->own_allowed_path) {  // Handle strdup failure
        AR__HEAP__FREE(own_delegate);  // Clean up delegate before returning
        return NULL;
    }

    return own_delegate;
}
```

**Verification:**
- Both dlsym tests pass
- No memory leaks even when malloc/strdup fail
- Makefile excludes `*_dlsym_tests.c` from sanitizer builds

---

#### Iteration 8.2: ar_file_delegate__destroy() cleans up without leaks - PENDING REVIEW

**Objective**: Verify FileDelegate destruction properly frees resources

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch memory leaks
- **Method**: Apply temporary corruption (skip cleanup), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what cleanup is needed
- **Method**: Observe what resources need freeing
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__destroy_cleans_up(void) {
    // Given a FileDelegate instance
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    // When destroying the delegate
    ar_file_delegate__destroy(own_delegate);

    // Then no memory should leak (verified by memory report)
    // No explicit assertion needed - memory leak check validates this

    // Cleanup
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Skip freeing the path
```c
void ar_file_delegate__destroy(ar_file_delegate_t *own_delegate) {
    if (!own_delegate) return;
    // TEMPORARY: Don't free own_allowed_path to cause leak
    // AR__HEAP__FREE(own_delegate->own_allowed_path);  // COMMENTED OUT
    AR__HEAP__FREE(own_delegate);
}
```
Expected RED: "Memory leak detected in test report"
Verify: `make ar_file_delegate_tests` → memory leak reported

---

**GREEN Phase:**

**This iteration**: NEW - Need to properly free all resources

```c
void ar_file_delegate__destroy(ar_file_delegate_t *own_delegate) {
    if (!own_delegate) return;

    AR__HEAP__FREE(own_delegate->own_allowed_path);  // Free the path
    AR__HEAP__FREE(own_delegate);
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 8.2.1: ar_file_delegate__destroy() handles NULL parameter safely - PENDING REVIEW

**Objective**: Verify destroy() safely handles NULL delegate parameter

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch unsafe NULL handling
- **Method**: Temporarily remove NULL check (causes crash), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__destroy_handles_null(void) {
    // Given a NULL delegate
    ar_file_delegate_t *own_delegate = NULL;

    // When destroying NULL delegate
    ar_file_delegate__destroy(own_delegate);

    // Then it should not crash (no assertion needed - just shouldn't crash)
    // Success = no segfault
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Remove NULL check to cause crash

```c
void ar_file_delegate__destroy(ar_file_delegate_t *own_delegate) {
    // TEMPORARY: Skip NULL check to prove test catches it (will crash)
    // if (!own_delegate) return;

    AR__HEAP__FREE(own_delegate->own_allowed_path);  // CRASH on NULL
    AR__HEAP__FREE(own_delegate);
}
```

Expected RED: "Test crashes with segmentation fault"
Verify: `make ar_file_delegate_tests` → segfault

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL check for safety

```c
void ar_file_delegate__destroy(ar_file_delegate_t *own_delegate) {
    if (!own_delegate) return;  // Safe NULL handling

    AR__HEAP__FREE(own_delegate->own_allowed_path);
    AR__HEAP__FREE(own_delegate);
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 8.3: ar_file_delegate__get_type() returns correct type - REVIEWED

**Objective**: Verify FileDelegate has a type identifier

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch wrong type returns
- **Method**: Apply temporary corruption (return wrong type), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what type identifier to use
- **Method**: Observe what test expects
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__get_type(void) {
    // Given a FileDelegate instance
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // When getting the delegate type
    const char *ref_type = ar_file_delegate__get_type(own_delegate);

    // Then it should return "file"
    AR_ASSERT(strcmp(ref_type, "file") == 0, "Type should be 'file'");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Return wrong type
```c
const char* ar_file_delegate__get_type(const ar_file_delegate_t *ref_delegate) {
    return "network";  // TEMPORARY: Wrong type to prove test catches it
}
```
Expected RED: "Test FAILS because type is 'network' not 'file'"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Return correct type identifier

```c
const char* ar_file_delegate__get_type(const ar_file_delegate_t *ref_delegate) {
    return "file";  // Hardcoded - minimal implementation
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

### TDD Cycle 9: File Read Operation

#### Iteration 9.0.1: ar_file_delegate__read() handles NULL delegate parameter - PENDING REVIEW

**Objective**: Verify read() returns NULL when delegate parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS (crashes)
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_handles_null_delegate(void) {
    // Given a NULL delegate
    ar_file_delegate_t *mut_delegate = NULL;

    // When attempting to read with NULL delegate
    ar_data_t *own_content = ar_file_delegate__read(mut_delegate, "/tmp/test.txt");

    // Then it should return NULL (not crash)
    AR_ASSERT(own_content == NULL, "Should reject NULL delegate");  // ← FAILS (crashes)

    // Cleanup (none needed)
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation doesn't exist yet, so will crash on NULL dereference.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL delegate validation

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;  // Handle NULL delegate

    // Minimal: Just return NULL for now
    return NULL;
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 9.0.2: ar_file_delegate__read() handles NULL path parameter - PENDING REVIEW

**Objective**: Verify read() returns NULL when path parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS (crashes)
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_handles_null_path(void) {
    // Given a valid delegate but NULL path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // When attempting to read with NULL path
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, NULL);

    // Then it should return NULL (not crash)
    AR_ASSERT(own_content == NULL, "Should reject NULL path");  // ← FAILS (crashes)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation (from 9.0.1) doesn't check path for NULL, so will crash.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL path validation

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;
    if (!ref_path) return NULL;  // Handle NULL path

    // Minimal: Just return NULL for now
    return NULL;
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 9.1: ar_file_delegate__read() returns NULL when file doesn't exist - REVIEWED

**Objective**: Verify read operation fails gracefully for non-existent files

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch incorrect success returns
- **Method**: Apply temporary corruption (return fake data), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what to return for missing files
- **Method**: Observe test expectation
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_returns_null_when_missing(void) {
    // Given a FileDelegate and non-existent file path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");
    const char *ref_path = "/tmp/allowed/nonexistent.txt";

    // When attempting to read the file
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);

    // Then it should return NULL
    AR_ASSERT(own_content == NULL, "Should return NULL for missing file");  // ← FAILS (stub returns data)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Return fake data instead of NULL
```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    // TEMPORARY: Return fake data to prove test catches it
    return ar_data__create_string("fake content");
}
```
Expected RED: "Test FAILS because content is not NULL"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Return NULL for missing files

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;  // From 9.0.1
    if (!ref_path) return NULL;      // From 9.0.2

    // Check if file exists using ar_io
    if (!ar_io__file_exists(ref_path)) {
        return NULL;  // File doesn't exist
    }

    return NULL;  // Minimal: For existing files, still return NULL (9.2 will add actual read)
}
```

**Note**: The final `return NULL` is intentional minimalism - iteration 9.2 will force actual file reading.

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 9.2.1: ar_file_delegate__read() returns non-NULL when file exists - REVIEWED

**Objective**: Verify read operation returns data when file exists

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch wrong content returns
- **Method**: Apply temporary corruption (return wrong content), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to read file content
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_returns_non_null_when_exists(void) {
    // Given a FileDelegate and an existing file
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Create a test file
    const char *ref_path = "/tmp/test_file.txt";
    const char *ref_content = "Hello, World!";
    ar_io__write_file(ref_path, ref_content, strlen(ref_content));

    // When reading the file
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);

    // Then it should return non-NULL
    AR_ASSERT(own_content != NULL, "Should return content");  // ← FAILS (returns NULL from 9.1)

    // Cleanup
    ar_data__destroy(own_content);
    ar_file_delegate__destroy(own_delegate);
    ar_io__delete_file(ref_path);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation (from 9.1) returns NULL for all files, so test will fail.

---

**GREEN Phase:**

**This iteration**: NEW - Actually read and return file content

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;  // From 9.0.1
    if (!ref_path) return NULL;      // From 9.0.2

    if (!ar_io__file_exists(ref_path)) {
        return NULL;  // From 9.1
    }

    // Read file content using ar_io
    size_t size = 0;
    char *own_content = ar_io__read_file(ref_path, &size);
    if (!own_content) return NULL;

    // Create data object (transfers ownership)
    ar_data_t *own_data = ar_data__create_string(own_content);
    AR__HEAP__FREE(own_content);

    return own_data;  // Non-NULL for existing files
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 9.2.2: ar_file_delegate__read() returns correct content - REVIEWED

**Objective**: Verify read operation returns the actual file content

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch wrong content returns
- **Method**: Apply temporary corruption (return wrong content), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify content integrity
- **Method**: Already implemented in 9.2.1
- **Status**: ✅ ALREADY SATISFIED - Implemented in Iteration 9.2.1

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_returns_correct_content(void) {
    // Given a FileDelegate and an existing file
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Create a test file
    const char *ref_path = "/tmp/test_content.txt";
    const char *ref_content = "Hello, World!";
    ar_io__write_file(ref_path, ref_content, strlen(ref_content));

    // When reading the file
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);
    AR_ASSERT(own_content != NULL, "Setup: should return content");

    // Then content should match
    AR_ASSERT(strcmp(ar_data__get_string(own_content), ref_content) == 0,
              "Content should match");  // ← Verify correct content

    // Cleanup
    ar_data__destroy(own_content);
    ar_file_delegate__destroy(own_delegate);
    ar_io__delete_file(ref_path);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Return wrong content
```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;
    if (!ref_path) return NULL;
    if (!ar_io__file_exists(ref_path)) return NULL;

    // TEMPORARY: Return wrong content to prove test catches it
    return ar_data__create_string("wrong content");
}
```
Expected RED: "Test FAILS because content doesn't match"
Verify: `make ar_file_delegate_tests` → assertion fails on content match

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Remove temporary corruption

Since Goal 2 is already satisfied (content reading implemented in 9.2.1), we just need to remove the temporary corruption.

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;
    if (!ref_path) return NULL;
    if (!ar_io__file_exists(ref_path)) return NULL;

    // Read file content using ar_io (from 9.2.1)
    size_t size = 0;
    char *own_content = ar_io__read_file(ref_path, &size);
    if (!own_content) return NULL;

    // Create data object (transfers ownership)
    ar_data_t *own_data = ar_data__create_string(own_content);
    AR__HEAP__FREE(own_content);

    return own_data;  // Returns correct content
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 9.3: Ownership of read data transfers to caller - REVIEWED

**Objective**: Verify caller must destroy returned data (ownership verification)

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch ownership issues
- **Method**: Skip destroying returned data, verify memory leak
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify ownership semantics
- **Method**: Already implemented in 9.2
- **Status**: ✅ ALREADY SATISFIED - Implemented in Iteration 9.2

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_transfers_ownership(void) {
    // Given a FileDelegate and existing file
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    const char *ref_path = "/tmp/test_ownership.txt";
    ar_io__write_file(ref_path, "test", 4);

    // When reading the file multiple times
    ar_data_t *own_content1 = ar_file_delegate__read(own_delegate, ref_path);
    ar_data_t *own_content2 = ar_file_delegate__read(own_delegate, ref_path);

    // Then each read returns independent owned data
    AR_ASSERT(own_content1 != NULL, "First read should succeed");
    AR_ASSERT(own_content2 != NULL, "Second read should succeed");
    AR_ASSERT(own_content1 != own_content2, "Should be separate instances");

    // Cleanup - MUST destroy both (ownership test)
    ar_data__destroy(own_content1);
    ar_data__destroy(own_content2);
    ar_file_delegate__destroy(own_delegate);
    ar_io__delete_file(ref_path);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Comment out cleanup to cause memory leak
```c
// In test, temporarily comment out:
// ar_data__destroy(own_content1);  // TEMPORARY: Skip to prove ownership
// ar_data__destroy(own_content2);  // TEMPORARY: Skip to prove ownership
```
Expected RED: "Memory leak detected - 2 ar_data_t objects not freed"
Verify: `make ar_file_delegate_tests` → memory leak report shows leaks

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Just remove temporary corruption

Since Goal 2 is already satisfied (ownership transfer implemented in 9.2), we just need to remove the temporary corruption (uncommenting the destroy calls).

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

### TDD Cycle 10: Path Validation

#### Iteration 10.1: Read rejects paths with "../" - REVIEWED

**Objective**: Verify directory traversal attempts are blocked

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch security bypasses
- **Method**: Temporarily allow "../" paths, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to validate paths
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_rejects_directory_traversal(void) {
    // Given a FileDelegate with restricted path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // When attempting directory traversal
    const char *ref_evil_path = "/tmp/allowed/../../../etc/passwd";
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_evil_path);

    // Then it should return NULL (rejected)
    AR_ASSERT(own_content == NULL, "Should reject directory traversal");  // ← FAILS (not checking)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Allow "../" paths to pass through
```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate || !ref_path) return NULL;

    // TEMPORARY: Skip path validation to prove test catches it
    // if (strstr(ref_path, "../")) return NULL;  // COMMENTED OUT

    if (!ar_io__file_exists(ref_path)) return NULL;

    // ... rest of read implementation
}
```
Expected RED: "Test FAILS - traversal path not rejected"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Add path validation

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate || !ref_path) return NULL;

    // Path validation - reject directory traversal
    if (strstr(ref_path, "../")) {
        ar_log__error(mut_delegate->ref_log, "Path contains directory traversal: %s", ref_path);
        return NULL;
    }

    if (!ar_io__file_exists(ref_path)) return NULL;

    // ... rest of read implementation
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 10.2: Read accepts valid relative paths - REVIEWED

**Objective**: Verify legitimate relative paths work

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch over-restrictive validation
- **Method**: Temporarily reject all relative paths, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what makes a path valid
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_accepts_relative_paths(void) {
    // Given a FileDelegate and valid relative path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Create file with relative path
    const char *ref_path = "test_subdir/file.txt";
    ar_io__create_directory("/tmp/test_subdir");
    ar_io__write_file("/tmp/test_subdir/file.txt", "content", 7);

    // When reading with relative path
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);

    // Then it should succeed
    AR_ASSERT(own_content != NULL, "Should accept relative path");  // ← FAILS (path not resolved)

    // Cleanup
    ar_data__destroy(own_content);
    ar_file_delegate__destroy(own_delegate);
    ar_io__delete_file("/tmp/test_subdir/file.txt");
    ar_io__remove_directory("/tmp/test_subdir");
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Reject all paths without '/'
```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate || !ref_path) return NULL;

    if (strstr(ref_path, "../")) return NULL;

    // TEMPORARY: Reject relative paths to prove test catches it
    if (ref_path[0] != '/') return NULL;

    // ... rest
}
```
Expected RED: "Test FAILS - relative path rejected"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Support path resolution

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate || !ref_path) return NULL;

    if (strstr(ref_path, "../")) return NULL;

    // Build full path for relative paths
    char full_path[PATH_MAX];
    if (ref_path[0] != '/') {
        snprintf(full_path, PATH_MAX, "%s/%s",
                 mut_delegate->own_allowed_path, ref_path);
    } else {
        strncpy(full_path, ref_path, PATH_MAX);
    }

    if (!ar_io__file_exists(full_path)) return NULL;

    // Read using full path
    size_t size = 0;
    char *own_content = ar_io__read_file(full_path, &size);
    if (!own_content) return NULL;

    ar_data_t *own_data = ar_data__create_string(own_content);
    AR__HEAP__FREE(own_content);

    return own_data;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 10.3: Read accepts absolute paths within allowed directories - REVIEWED

**Objective**: Verify absolute paths are allowed if within boundaries

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch path restriction bypasses
- **Method**: Temporarily reject all absolute paths, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to check path boundaries
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_accepts_absolute_within_allowed(void) {
    // Given a FileDelegate with allowed path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // Create file in allowed directory
    ar_io__create_directory("/tmp/allowed");
    const char *ref_path = "/tmp/allowed/file.txt";
    ar_io__write_file(ref_path, "content", 7);

    // When reading with absolute path within allowed directory
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);

    // Then it should succeed
    AR_ASSERT(own_content != NULL, "Should accept absolute path within allowed");  // ← FAILS

    // Cleanup
    ar_data__destroy(own_content);
    ar_file_delegate__destroy(own_delegate);
    ar_io__delete_file(ref_path);
    ar_io__remove_directory("/tmp/allowed");
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Reject absolute paths outside current implementation
```c
// In read function, temporarily:
if (ref_path[0] == '/' &&
    strncmp(ref_path, mut_delegate->own_allowed_path,
            strlen(mut_delegate->own_allowed_path)) != 0) {
    return NULL;  // TEMPORARY: Too restrictive
}
```
Expected RED: "Test FAILS - absolute path rejected"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Check path boundaries properly

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate || !ref_path) return NULL;

    // Reject directory traversal
    if (strstr(ref_path, "../")) return NULL;

    char full_path[PATH_MAX];

    // Handle absolute vs relative paths
    if (ref_path[0] == '/') {
        // Absolute path - must be within allowed directory
        if (strncmp(ref_path, mut_delegate->own_allowed_path,
                    strlen(mut_delegate->own_allowed_path)) != 0) {
            ar_log__error(mut_delegate->ref_log,
                         "Path outside allowed directory: %s", ref_path);
            return NULL;
        }
        strncpy(full_path, ref_path, PATH_MAX);
    } else {
        // Relative path - prepend allowed directory
        snprintf(full_path, PATH_MAX, "%s/%s",
                 mut_delegate->own_allowed_path, ref_path);
    }

    // Read file
    if (!ar_io__file_exists(full_path)) return NULL;

    size_t size = 0;
    char *own_content = ar_io__read_file(full_path, &size);
    if (!own_content) return NULL;

    ar_data_t *own_data = ar_data__create_string(own_content);
    AR__HEAP__FREE(own_content);

    return own_data;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

### TDD Cycle 11: File Write Operation

#### Iteration 11.0.1: ar_file_delegate__write() handles NULL delegate parameter - PENDING REVIEW

**Objective**: Verify write() returns false when delegate parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS (crashes)
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_handles_null_delegate(void) {
    // Given a NULL delegate
    ar_file_delegate_t *mut_delegate = NULL;

    // When attempting to write with NULL delegate
    ar_data_t *ref_content = ar_data__create_string("test");
    bool result = ar_file_delegate__write(mut_delegate, "/tmp/test.txt", ref_content);

    // Then it should return false (not crash)
    AR_ASSERT(!result, "Should reject NULL delegate");  // ← FAILS (crashes)

    // Cleanup
    ar_data__destroy(ref_content);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation doesn't exist yet, so will crash on NULL dereference.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL delegate validation

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate) return false;  // Handle NULL delegate

    // Minimal: Just return false for now
    return false;
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 11.0.2: ar_file_delegate__write() handles NULL path parameter - PENDING REVIEW

**Objective**: Verify write() returns false when path parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS (crashes)
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_handles_null_path(void) {
    // Given a valid delegate but NULL path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // When attempting to write with NULL path
    ar_data_t *ref_content = ar_data__create_string("test");
    bool result = ar_file_delegate__write(own_delegate, NULL, ref_content);

    // Then it should return false (not crash)
    AR_ASSERT(!result, "Should reject NULL path");  // ← FAILS (crashes)

    // Cleanup
    ar_data__destroy(ref_content);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation (from 11.0.1) doesn't check path for NULL, so will crash.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL path validation

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate) return false;  // From 11.0.1
    if (!ref_path) return false;      // Handle NULL path

    // Minimal: Just return false for now
    return false;
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 11.0.3: ar_file_delegate__write() handles NULL content parameter - PENDING REVIEW

**Objective**: Verify write() returns false when content parameter is NULL

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch missing NULL validation
- **Method**: Temporarily skip NULL check, verify test FAILS (crashes)
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_handles_null_content(void) {
    // Given a valid delegate but NULL content
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // When attempting to write with NULL content
    bool result = ar_file_delegate__write(own_delegate, "/tmp/test.txt", NULL);

    // Then it should return false (not crash)
    AR_ASSERT(!result, "Should reject NULL content");  // ← FAILS (crashes)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation (from 11.0.2) doesn't check content for NULL, so will crash.

---

**GREEN Phase:**

**This iteration**: NEW - Add NULL content validation

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate) return false;  // From 11.0.1
    if (!ref_path) return false;      // From 11.0.2
    if (!ref_content) return false;   // Handle NULL content

    // Minimal: Just return false for now
    return false;
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 11.1: ar_file_delegate__write() returns false for invalid path - REVIEWED

**Objective**: Verify write operation rejects invalid paths

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch security bypasses
- **Method**: Temporarily allow invalid paths, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine write validation logic
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_rejects_invalid_path(void) {
    // Given a FileDelegate and invalid path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // When attempting to write with directory traversal
    const char *ref_path = "/tmp/allowed/../../../etc/passwd";
    ar_data_t *ref_content = ar_data__create_string("evil");
    bool result = ar_file_delegate__write(own_delegate, ref_path, ref_content);

    // Then it should return false
    AR_ASSERT(!result, "Should reject invalid path");  // ← FAILS (stub returns true)

    // Cleanup
    ar_data__destroy(ref_content);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Always return true
```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    // TEMPORARY: Always succeed to prove test catches it
    return true;
}
```
Expected RED: "Test FAILS - invalid path not rejected"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Add path validation for write

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate) return false;  // From 11.0.1
    if (!ref_path) return false;      // From 11.0.2
    if (!ref_content) return false;   // From 11.0.3

    // Reject directory traversal
    if (strstr(ref_path, "../")) {
        ar_log__error(mut_delegate->ref_log,
                     "Write path contains directory traversal: %s", ref_path);
        return false;
    }

    return false;  // Minimal: Always fail for now (11.2 will add actual write)
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 11.2.1: ar_file_delegate__write() returns true for valid relative path - REVIEWED

**Objective**: Verify write operation succeeds for valid relative paths

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch write failures
- **Method**: Current implementation returns false (from 11.1), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to write files with relative paths
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_succeeds_with_relative_path(void) {
    // Given a FileDelegate and valid relative path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // When writing to a file with relative path
    const char *ref_path = "test_write.txt";
    ar_data_t *ref_content = ar_data__create_string("Hello");
    bool result = ar_file_delegate__write(own_delegate, ref_path, ref_content);

    // Then it should return true
    AR_ASSERT(result, "Write should succeed");  // ← FAILS (returns false from 11.1)

    // Cleanup
    ar_data__destroy(ref_content);
    ar_io__delete_file("/tmp/test_write.txt");
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Already proven - current implementation always returns false from 11.1

---

**GREEN Phase:**

**This iteration**: NEW - Implement write with relative path support

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate) return false;  // From 11.0.1
    if (!ref_path) return false;      // From 11.0.2
    if (!ref_content) return false;   // From 11.0.3

    // Reject directory traversal (from 11.1)
    if (strstr(ref_path, "../")) {
        ar_log__error(mut_delegate->ref_log,
                     "Write path contains directory traversal: %s", ref_path);
        return false;
    }

    // Build full path for relative paths
    char full_path[PATH_MAX];
    if (ref_path[0] != '/') {
        snprintf(full_path, PATH_MAX, "%s/%s",
                 mut_delegate->own_allowed_path, ref_path);
    } else {
        // Absolute path - for now just copy (11.2.2 will add validation)
        strncpy(full_path, ref_path, PATH_MAX);
    }

    // Write file using ar_io
    const char *ref_str = ar_data__get_string(ref_content);
    if (!ref_str) return false;

    return ar_io__write_file(full_path, ref_str, strlen(ref_str));
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 11.2.2: ar_file_delegate__write() accepts absolute path within allowed - REVIEWED

**Objective**: Verify write operation accepts absolute paths within allowed directory

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch incorrect path rejection
- **Method**: Temporarily reject all absolute paths, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine absolute path boundary checking
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_accepts_absolute_within_allowed(void) {
    // Given a FileDelegate with allowed path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // Create allowed directory
    ar_io__create_directory("/tmp/allowed");

    // When writing with absolute path within allowed directory
    const char *ref_path = "/tmp/allowed/file.txt";
    ar_data_t *ref_content = ar_data__create_string("content");
    bool result = ar_file_delegate__write(own_delegate, ref_path, ref_content);

    // Then it should succeed
    AR_ASSERT(result, "Should accept absolute path within allowed");  // ← FAILS

    // Cleanup
    ar_data__destroy(ref_content);
    ar_io__delete_file(ref_path);
    ar_io__remove_directory("/tmp/allowed");
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Reject all absolute paths

```c
// In write function, temporarily:
if (ref_path[0] == '/') {
    return false;  // TEMPORARY: Reject all absolute paths
}
```

Expected RED: "Test FAILS - absolute path rejected"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: NEW - Add absolute path boundary checking

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate) return false;
    if (!ref_path) return false;
    if (!ref_content) return false;

    // Reject directory traversal
    if (strstr(ref_path, "../")) {
        ar_log__error(mut_delegate->ref_log,
                     "Write path contains directory traversal: %s", ref_path);
        return false;
    }

    // Build full path
    char full_path[PATH_MAX];
    if (ref_path[0] == '/') {
        // Absolute path - must be within allowed directory
        if (strncmp(ref_path, mut_delegate->own_allowed_path,
                    strlen(mut_delegate->own_allowed_path)) != 0) {
            ar_log__error(mut_delegate->ref_log,
                         "Path outside allowed directory: %s", ref_path);
            return false;
        }
        strncpy(full_path, ref_path, PATH_MAX);
    } else {
        // Relative path - prepend allowed directory
        snprintf(full_path, PATH_MAX, "%s/%s",
                 mut_delegate->own_allowed_path, ref_path);
    }

    // Write file using ar_io
    const char *ref_str = ar_data__get_string(ref_content);
    if (!ref_str) return false;

    return ar_io__write_file(full_path, ref_str, strlen(ref_str));
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 11.2.3: ar_file_delegate__write() rejects absolute path outside allowed - REVIEWED

**Objective**: Verify write operation rejects absolute paths outside allowed directory

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch security bypass
- **Method**: Temporarily allow all absolute paths, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify boundary checking works
- **Method**: Already implemented in 11.2.2
- **Status**: ✅ ALREADY SATISFIED - Implemented in Iteration 11.2.2

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_rejects_absolute_outside_allowed(void) {
    // Given a FileDelegate with restricted path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // When attempting to write outside allowed directory
    const char *ref_path = "/tmp/evil/file.txt";
    ar_data_t *ref_content = ar_data__create_string("evil");
    bool result = ar_file_delegate__write(own_delegate, ref_path, ref_content);

    // Then it should return false
    AR_ASSERT(!result, "Should reject path outside allowed");  // ← Verify rejection

    // Cleanup
    ar_data__destroy(ref_content);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Skip boundary check

```c
// In write function, temporarily:
if (ref_path[0] == '/') {
    // TEMPORARY: Skip boundary check - accept all absolute paths
    // if (strncmp(ref_path, mut_delegate->own_allowed_path, ...) != 0) {
    //     return false;
    // }
    strncpy(full_path, ref_path, PATH_MAX);
}
```

Expected RED: "Test FAILS - path outside allowed directory not rejected"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Remove temporary corruption

Since Goal 2 is already satisfied (boundary checking implemented in 11.2.2), just remove the temporary corruption.

```c
// Proper boundary check (from 11.2.2 - remove temporary bypass)
if (ref_path[0] == '/') {
    if (strncmp(ref_path, mut_delegate->own_allowed_path,
                strlen(mut_delegate->own_allowed_path)) != 0) {
        ar_log__error(mut_delegate->ref_log,
                     "Path outside allowed directory: %s", ref_path);
        return false;  // Properly reject
    }
    strncpy(full_path, ref_path, PATH_MAX);
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 11.3: Written file contains correct content - REVIEWED

**Objective**: Verify written content is preserved correctly

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch content corruption
- **Method**: Write wrong content, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify content integrity
- **Method**: Already implemented in 11.2
- **Status**: ✅ ALREADY SATISFIED - Implemented in Iteration 11.2

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_preserves_content(void) {
    // Given a FileDelegate
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // When writing content to a file
    const char *ref_path = "test_content.txt";
    const char *ref_expected = "Line 1\nLine 2\nLine 3";
    ar_data_t *ref_content = ar_data__create_string(ref_expected);
    ar_file_delegate__write(own_delegate, ref_path, ref_content);

    // Then reading back should match
    ar_data_t *own_read = ar_file_delegate__read(own_delegate, ref_path);
    AR_ASSERT(own_read != NULL, "Should read back");
    AR_ASSERT(strcmp(ar_data__get_string(own_read), ref_expected) == 0,
              "Content should match");  // ← Verify content preserved

    // Cleanup
    ar_data__destroy(ref_content);
    ar_data__destroy(own_read);
    ar_io__delete_file("/tmp/test_content.txt");
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Modify write to corrupt content

```c
// In write function, temporarily replace the actual write with:
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    // ... all validation code stays the same ...

    // TEMPORARY: Write corrupted content instead of actual content
    return ar_io__write_file(full_path, "corrupted", 9);

    // Original code (commented out temporarily):
    // const char *ref_str = ar_data__get_string(ref_content);
    // if (!ref_str) return false;
    // return ar_io__write_file(full_path, ref_str, strlen(ref_str));
}
```

Expected RED: "Test FAILS - content doesn't match (expected 'Line 1\nLine 2\nLine 3', got 'corrupted')"
Verify: `make ar_file_delegate_tests` → assertion fails on content match

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Remove temporary corruption

Goal 2 already satisfied by 11.2 implementation. Just remove temporary corruption.

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

### TDD Cycle 12: File Size Limits

#### Iteration 12.0.1: ar_file_delegate__set_max_size() accepts valid parameters - REVIEWED

**Objective**: Verify setter function can be called without crashing

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch function not existing
- **Method**: Comment out function, verify test FAILS (compilation error)
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine setter signature and basic implementation
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__set_max_size_accepts_valid_params(void) {
    // Given a FileDelegate
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    // When setting max size with valid parameters
    ar_file_delegate__set_max_size(own_delegate, 100);
    ar_file_delegate__set_max_size(own_delegate, 200);  // Change it
    ar_file_delegate__set_max_size(own_delegate, 0);    // Set to unlimited

    // Then it should not crash
    // (Actual effectiveness verified in 12.1)
    AR_ASSERT(true, "Setter should not crash with valid params");  // ← FAILS if function doesn't exist

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Comment out the setter function

```c
// TEMPORARY: Comment out setter to prove test catches it
// void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
//     if (!mut_delegate) return;
//     mut_delegate->max_size = max_size;
// }
```

Expected RED: "Compilation error - undefined function ar_file_delegate__set_max_size"
Verify: `make ar_file_delegate_tests` → compilation fails

---

**GREEN Phase:**

**This iteration**: NEW - Add max_size field and setter function

```c
// Update struct to include max_size field
typedef struct ar_file_delegate_s {
    ar_log_t *ref_log;
    char *own_allowed_path;
    size_t max_size;  // Add size limit field (0 = unlimited)
} ar_file_delegate_t;

// Add setter function
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (!mut_delegate) return;  // NULL handling (not yet tested - see 12.0.2)
    mut_delegate->max_size = max_size;
}

// Update create() to initialize max_size to 0 (unlimited)
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    // ... existing code ...
    own_delegate->max_size = 0;  // Default: unlimited
    return own_delegate;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 12.0.2: ar_file_delegate__set_max_size() handles NULL delegate safely - REVIEWED

**Objective**: Verify setter safely handles NULL delegate parameter

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch unsafe NULL handling
- **Method**: Temporarily remove NULL check (causes crash), verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine NULL handling behavior
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__set_max_size_handles_null(void) {
    // Given a NULL delegate
    ar_file_delegate_t *mut_delegate = NULL;

    // When calling setter with NULL delegate
    ar_file_delegate__set_max_size(mut_delegate, 100);

    // Then it should not crash (no assertion needed - just shouldn't crash)
    // Success = no segfault
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Remove NULL check to cause crash

```c
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    // TEMPORARY: Skip NULL check to prove test catches it (will crash)
    // if (!mut_delegate) return;

    mut_delegate->max_size = max_size;  // CRASH on NULL
}
```

Expected RED: "Test crashes with segmentation fault"
Verify: `make ar_file_delegate_tests` → segfault

---

**GREEN Phase:**

**This iteration**: Already implemented in 12.0.1 - just verify it works

The NULL check was already added in 12.0.1's GREEN phase:

```c
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (!mut_delegate) return;  // Safe NULL handling (from 12.0.1)
    mut_delegate->max_size = max_size;
}
```

**Verification:**
- Test passes (no crash)
- Memory leak check: 0 leaks

---

#### Iteration 12.1: Read fails when file exceeds size limit - REVIEWED

**Objective**: Verify read operation respects size limits

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch size limit bypasses
- **Method**: Temporarily ignore size limit, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to enforce size limits
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__read_respects_size_limit(void) {
    // Given a FileDelegate with size limit
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");
    ar_file_delegate__set_max_size(own_delegate, 100);  // 100 byte limit

    // Create large file
    const char *ref_path = "/tmp/large.txt";
    char large_content[200];
    memset(large_content, 'A', 199);
    large_content[199] = '\0';
    ar_io__write_file(ref_path, large_content, 200);

    // When reading large file
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);

    // Then it should return NULL (too large)
    AR_ASSERT(own_content == NULL, "Should reject large file");  // ← FAILS (no limit check)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_io__delete_file(ref_path);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current implementation doesn't check size, so test will fail initially.

---

**GREEN Phase:**

**This iteration**: NEW - Add size checking to read()

```c
// Update read to check size (struct field and setter from 12.0.1)
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate) return NULL;
    if (!ref_path) return NULL;

    // Reject directory traversal
    if (strstr(ref_path, "../")) return NULL;

    // Build full path (logic from 10.2, 10.3)
    char full_path[PATH_MAX];
    // ... path resolution code ...

    // NEW: Check file size before reading
    if (mut_delegate->max_size > 0) {
        size_t file_size = ar_io__get_file_size(full_path);
        if (file_size > mut_delegate->max_size) {
            ar_log__error(mut_delegate->ref_log,
                         "File exceeds size limit: %zu > %zu",
                         file_size, mut_delegate->max_size);
            return NULL;
        }
    }

    // Existing read logic (from 9.2.1)
    if (!ar_io__file_exists(full_path)) return NULL;

    size_t size = 0;
    char *own_content = ar_io__read_file(full_path, &size);
    if (!own_content) return NULL;

    ar_data_t *own_data = ar_data__create_string(own_content);
    AR__HEAP__FREE(own_content);

    return own_data;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 12.2: Write fails when content exceeds size limit - REVIEWED

**Objective**: Verify write operation respects size limits

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch size limit bypasses on write
- **Method**: Temporarily ignore size on write, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to check content size
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_respects_size_limit(void) {
    // Given a FileDelegate with size limit
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");
    ar_file_delegate__set_max_size(own_delegate, 50);  // 50 byte limit

    // When writing large content
    char large_content[100];
    memset(large_content, 'B', 99);
    large_content[99] = '\0';
    ar_data_t *ref_content = ar_data__create_string(large_content);
    bool result = ar_file_delegate__write(own_delegate, "large.txt", ref_content);

    // Then it should return false (too large)
    AR_ASSERT(!result, "Should reject large content");  // ← FAILS (no size check on write)

    // Cleanup
    ar_data__destroy(ref_content);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

Current write doesn't check size, so test will fail.

---

**GREEN Phase:**

**This iteration**: NEW - Add size checking to write

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    // ... existing validation ...

    // Check content size
    const char *ref_str = ar_data__get_string(ref_content);
    if (!ref_str) return false;

    size_t content_size = strlen(ref_str);
    if (mut_delegate->max_size > 0 && content_size > mut_delegate->max_size) {
        ar_log__error(mut_delegate->ref_log,
                     "Content exceeds size limit: %zu > %zu",
                     content_size, mut_delegate->max_size);
        return false;
    }

    // ... rest of write implementation ...
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 12.3: Size limit is configurable - PENDING REVIEW

**Objective**: Verify size limit can be changed and 0 means unlimited

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test verifies configurability
- **Method**: Hard-code size limit, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify setter works and 0 means unlimited
- **Method**: Already partially implemented
- **Status**: Needs enhancement

---

**Step 1: Write the Test**

```c
static void test_file_delegate__size_limit_configurable(void) {
    // Given a FileDelegate
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Initially no limit (0 = unlimited)
    char content[200];
    memset(content, 'C', 199);
    content[199] = '\0';
    ar_data_t *ref_content = ar_data__create_string(content);

    // When no limit set, large content should work
    bool result1 = ar_file_delegate__write(own_delegate, "test1.txt", ref_content);
    AR_ASSERT(result1, "Should accept with no limit");

    // When limit set to 100, should fail
    ar_file_delegate__set_max_size(own_delegate, 100);
    bool result2 = ar_file_delegate__write(own_delegate, "test2.txt", ref_content);
    AR_ASSERT(!result2, "Should reject with 100 byte limit");

    // When limit changed to 300, should succeed
    ar_file_delegate__set_max_size(own_delegate, 300);
    bool result3 = ar_file_delegate__write(own_delegate, "test3.txt", ref_content);
    AR_ASSERT(result3, "Should accept with 300 byte limit");

    // Cleanup
    ar_data__destroy(ref_content);
    ar_io__delete_file("/tmp/test1.txt");
    ar_io__delete_file("/tmp/test3.txt");
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Hard-code size limit
```c
// In setter, temporarily:
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (mut_delegate) {
        mut_delegate->max_size = 50;  // TEMPORARY: Hard-coded, ignores parameter
    }
}
```
Expected RED: "Test FAILS - configurability broken"
Verify: `make ar_file_delegate_tests` → assertions fail

---

**GREEN Phase:**

**This iteration**: Fix setter and ensure 0 means unlimited (already mostly done)

```c
// Proper setter (remove temporary hard-coding)
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (mut_delegate) {
        mut_delegate->max_size = max_size;
    }
}

// In create, initialize to 0 (unlimited)
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path) {
    // ... existing create code ...
    own_delegate->max_size = 0;  // Default: unlimited
    // ...
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

**NOTE**: This iteration has multiple assertions and violates Lesson 2. See revised iterations 12.3.1, 12.3.2, 12.3.3 below for proper one-assertion-per-iteration structure.

---

#### Iteration 12.3.1: Size limit 0 means unlimited (allows large content) - REVIEWED

**Objective**: Verify size limit 0 (default) means no size restrictions

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch incorrect unlimited behavior
- **Method**: Temporarily enforce limit even when 0, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify 0 means unlimited
- **Method**: Already implemented in size checking logic
- **Status**: ✅ ALREADY SATISFIED - Implemented in 12.1 and 12.2 (check `if (max_size > 0)`)

---

**Step 1: Write the Test**

```c
static void test_file_delegate__size_limit_zero_means_unlimited(void) {
    // Given a FileDelegate with default size limit (0 = unlimited)
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Create large content (200 bytes)
    char content[200];
    memset(content, 'A', 199);
    content[199] = '\0';
    ar_data_t *ref_content = ar_data__create_string(content);

    // When writing large content with no limit set
    bool result = ar_file_delegate__write(own_delegate, "test_unlimited.txt", ref_content);

    // Then it should succeed (unlimited)
    AR_ASSERT(result, "Should accept large content when limit is 0");  // ← Verify unlimited

    // Cleanup
    ar_data__destroy(ref_content);
    ar_io__delete_file("/tmp/test_unlimited.txt");
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Remove the `max_size > 0` check to always enforce limit

```c
// In write function, temporarily remove the check for 0:
bool ar_file_delegate__write(...) {
    // ... validation ...

    const char *ref_str = ar_data__get_string(ref_content);
    if (!ref_str) return false;
    size_t content_size = strlen(ref_str);

    // TEMPORARY: Always check size, even when max_size is 0
    if (content_size > mut_delegate->max_size) {  // Bug: doesn't check max_size > 0
        ar_log__error(mut_delegate->ref_log,
                     "Content exceeds size limit: %zu > %zu",
                     content_size, mut_delegate->max_size);
        return false;
    }

    // ... rest of write ...
}
```

Expected RED: "Test FAILS - large content rejected even though limit is 0"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Confirm existing implementation

The `if (max_size > 0)` check was already implemented in 12.2. Just verify it works:

```c
// Correct implementation (from 12.2 - remove temporary bug)
if (mut_delegate->max_size > 0 && content_size > mut_delegate->max_size) {
    ar_log__error(mut_delegate->ref_log,
                 "Content exceeds size limit: %zu > %zu",
                 content_size, mut_delegate->max_size);
    return false;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 12.3.2: Size limit rejects content when limit is set - REVIEWED

**Objective**: Verify setting a limit actually enforces that limit

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch limit not being enforced
- **Method**: Temporarily ignore the limit, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify limit is enforced after being set
- **Method**: Already implemented in 12.2
- **Status**: ✅ ALREADY SATISFIED - Implemented in 12.2

---

**Step 1: Write the Test**

```c
static void test_file_delegate__size_limit_enforced_when_set(void) {
    // Given a FileDelegate with a size limit
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");
    ar_file_delegate__set_max_size(own_delegate, 100);  // Set 100 byte limit

    // Create large content (200 bytes - exceeds limit)
    char content[200];
    memset(content, 'B', 199);
    content[199] = '\0';
    ar_data_t *ref_content = ar_data__create_string(content);

    // When writing content that exceeds limit
    bool result = ar_file_delegate__write(own_delegate, "test_limited.txt", ref_content);

    // Then it should be rejected
    AR_ASSERT(!result, "Should reject content exceeding 100 byte limit");  // ← Verify rejection

    // Cleanup
    ar_data__destroy(ref_content);
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Ignore the size limit

```c
// In write function, temporarily skip the size check:
bool ar_file_delegate__write(...) {
    // ... validation ...

    const char *ref_str = ar_data__get_string(ref_content);
    if (!ref_str) return false;
    size_t content_size = strlen(ref_str);

    // TEMPORARY: Comment out size check to prove test catches it
    // if (mut_delegate->max_size > 0 && content_size > mut_delegate->max_size) {
    //     return false;
    // }

    return ar_io__write_file(full_path, ref_str, strlen(ref_str));
}
```

Expected RED: "Test FAILS - large content accepted even though limit is 100"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Confirm existing implementation from 12.2

```c
// Correct implementation (from 12.2 - remove temporary bypass)
const char *ref_str = ar_data__get_string(ref_content);
if (!ref_str) return false;
size_t content_size = strlen(ref_str);

if (mut_delegate->max_size > 0 && content_size > mut_delegate->max_size) {
    ar_log__error(mut_delegate->ref_log,
                 "Content exceeds size limit: %zu > %zu",
                 content_size, mut_delegate->max_size);
    return false;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 12.3.3: Size limit can be reconfigured (changed) - REVIEWED

**Objective**: Verify size limit can be changed to a different value

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch setter not updating the value
- **Method**: Temporarily ignore new value in setter, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Verify setter properly updates the limit
- **Method**: Already implemented in 12.0.1
- **Status**: ✅ ALREADY SATISFIED - Implemented in 12.0.1

---

**Step 1: Write the Test**

```c
static void test_file_delegate__size_limit_can_be_changed(void) {
    // Given a FileDelegate with initial limit of 100
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");
    ar_file_delegate__set_max_size(own_delegate, 100);

    // Create content that's 200 bytes (too large for 100 byte limit)
    char content[200];
    memset(content, 'C', 199);
    content[199] = '\0';
    ar_data_t *ref_content = ar_data__create_string(content);

    // When changing limit to 300 bytes
    ar_file_delegate__set_max_size(own_delegate, 300);

    // Then large content should now be accepted
    bool result = ar_file_delegate__write(own_delegate, "test_reconfigured.txt", ref_content);
    AR_ASSERT(result, "Should accept content after increasing limit to 300");  // ← Verify reconfiguration

    // Cleanup
    ar_data__destroy(ref_content);
    ar_io__delete_file("/tmp/test_reconfigured.txt");
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**Temporary corruption**: Setter ignores new value

```c
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (!mut_delegate) return;
    // TEMPORARY: Don't update - keep first value
    // mut_delegate->max_size = max_size;  // COMMENTED OUT
}
```

Expected RED: "Test FAILS - limit not updated, still 100"
Verify: `make ar_file_delegate_tests` → assertion fails

---

**GREEN Phase:**

**This iteration**: VERIFICATION - Confirm existing implementation from 12.0.1

```c
// Correct implementation (from 12.0.1 - remove temporary bypass)
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (!mut_delegate) return;
    mut_delegate->max_size = max_size;  // Properly update
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

## Related Patterns

- [TDD Iteration Planning Pattern](../kb/tdd-iteration-planning-pattern.md)
- [TDD GREEN Phase Minimalism](../kb/tdd-green-phase-minimalism.md)
- [TDD RED Phase Assertion Requirement](../kb/tdd-red-phase-assertion-requirement.md)
- [Red Phase Dual Goals Pattern](../kb/red-phase-dual-goals-pattern.md)
- [BDD Test Structure](../kb/bdd-test-structure.md)
- [Ownership Naming Conventions](../kb/ownership-naming-conventions.md)
- [Temporary Test Cleanup Pattern](../kb/temporary-test-cleanup-pattern.md)
- [Red-Green-Refactor Cycle](../kb/red-green-refactor-cycle.md)
- [TDD Plan Review Checklist](../kb/tdd-plan-review-checklist.md)
- [TDD Plan Iteration Split Pattern](../kb/tdd-plan-iteration-split-pattern.md)

---

**Plan Created**: 2025-10-18
**Feature**: FileDelegate Implementation
**Total Iterations**: 15 (5 cycles × 3 iterations)
**Status**: Ready for review with `/ar:review-plan`