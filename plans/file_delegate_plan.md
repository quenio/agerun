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
    if (!ref_log || !ref_allowed_path) return NULL;

    ar_file_delegate_t *own_delegate = AR__HEAP__MALLOC(sizeof(ar_file_delegate_t));
    if (!own_delegate) return NULL;

    own_delegate->ref_log = ref_log;
    own_delegate->own_allowed_path = AR__HEAP__STRDUP(ref_allowed_path);
    if (!own_delegate->own_allowed_path) {
        AR__HEAP__FREE(own_delegate);
        return NULL;
    }

    return own_delegate;  // Minimal: Just allocate and return
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

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

#### Iteration 8.3: ar_file_delegate__get_type() returns correct type - PENDING REVIEW

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

#### Iteration 9.1: ar_file_delegate__read() returns NULL when file doesn't exist - PENDING REVIEW

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
    if (!mut_delegate || !ref_path) return NULL;

    // Check if file exists using ar_io
    if (!ar_io__file_exists(ref_path)) {
        return NULL;  // File doesn't exist
    }

    return NULL;  // Minimal: Always return NULL for now
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 9.2: ar_file_delegate__read() returns content when file exists - PENDING REVIEW

**Objective**: Verify read operation returns file contents successfully

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
static void test_file_delegate__read_returns_content(void) {
    // Given a FileDelegate and an existing file
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Create a test file
    const char *ref_path = "/tmp/test_file.txt";
    const char *ref_content = "Hello, World!";
    ar_io__write_file(ref_path, ref_content, strlen(ref_content));

    // When reading the file
    ar_data_t *own_content = ar_file_delegate__read(own_delegate, ref_path);

    // Then it should return the content
    AR_ASSERT(own_content != NULL, "Should return content");  // ← FAILS (returns NULL from 9.1)
    AR_ASSERT(strcmp(ar_data__get_string(own_content), ref_content) == 0,
              "Content should match");

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
    if (!mut_delegate || !ref_path) return NULL;

    if (ar_io__file_exists(ref_path)) {
        // TEMPORARY: Return wrong content to prove test catches it
        return ar_data__create_string("wrong content");
    }

    return NULL;
}
```
Expected RED: "Test FAILS because content doesn't match"
Verify: `make ar_file_delegate_tests` → assertion fails on content match

---

**GREEN Phase:**

**This iteration**: NEW - Actually read and return file content

```c
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    if (!mut_delegate || !ref_path) return NULL;

    if (!ar_io__file_exists(ref_path)) {
        return NULL;
    }

    // Read file content using ar_io
    size_t size = 0;
    char *own_content = ar_io__read_file(ref_path, &size);
    if (!own_content) return NULL;

    // Create data object (transfers ownership)
    ar_data_t *own_data = ar_data__create_string(own_content);
    AR__HEAP__FREE(own_content);

    return own_data;
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 9.3: Ownership of read data transfers to caller - PENDING REVIEW

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

#### Iteration 10.1: Read rejects paths with "../" - PENDING REVIEW

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

#### Iteration 10.2: Read accepts valid relative paths - PENDING REVIEW

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

#### Iteration 10.3: Read accepts absolute paths within allowed directories - PENDING REVIEW

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

#### Iteration 11.1: ar_file_delegate__write() returns false for invalid path - PENDING REVIEW

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
    if (!mut_delegate || !ref_path || !ref_content) return false;

    // Reject directory traversal
    if (strstr(ref_path, "../")) {
        ar_log__error(mut_delegate->ref_log,
                     "Write path contains directory traversal: %s", ref_path);
        return false;
    }

    return false;  // Minimal: Always fail for now
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 11.2: ar_file_delegate__write() returns true when successful - PENDING REVIEW

**Objective**: Verify write operation succeeds for valid paths

**RED Phase:**

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED)
- **Purpose**: Prove this test can catch write failures
- **Method**: Temporarily always return false, verify test FAILS
- **Status**: Not yet proven

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine how to write files
- **Method**: Observe test expectations
- **Status**: Needs implementation

---

**Step 1: Write the Test**

```c
static void test_file_delegate__write_returns_true_on_success(void) {
    // Given a FileDelegate and valid path
    ar_log_t *ref_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // When writing to a file
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

**This iteration**: NEW - Implement actual write

```c
bool ar_file_delegate__write(ar_file_delegate_t *mut_delegate,
                             const char *ref_path,
                             ar_data_t *ref_content) {
    if (!mut_delegate || !ref_path || !ref_content) return false;

    // Reject directory traversal
    if (strstr(ref_path, "../")) {
        ar_log__error(mut_delegate->ref_log,
                     "Write path contains directory traversal: %s", ref_path);
        return false;
    }

    // Build full path
    char full_path[PATH_MAX];
    if (ref_path[0] != '/') {
        snprintf(full_path, PATH_MAX, "%s/%s",
                 mut_delegate->own_allowed_path, ref_path);
    } else {
        // Check absolute path is within allowed
        if (strncmp(ref_path, mut_delegate->own_allowed_path,
                    strlen(mut_delegate->own_allowed_path)) != 0) {
            return false;
        }
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

#### Iteration 11.3: Written file contains correct content - PENDING REVIEW

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
// In write function, temporarily:
// return ar_io__write_file(full_path, "corrupted", 9);  // TEMPORARY
```
Expected RED: "Test FAILS - content doesn't match"
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

#### Iteration 12.1: Read fails when file exceeds size limit - PENDING REVIEW

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

**This iteration**: NEW - Add size limit field and checking

```c
// Update struct
typedef struct ar_file_delegate_s {
    ar_log_t *ref_log;
    char *own_allowed_path;
    size_t max_size;  // Add size limit field
} ar_file_delegate_t;

// Add setter function
void ar_file_delegate__set_max_size(ar_file_delegate_t *mut_delegate, size_t max_size) {
    if (mut_delegate) {
        mut_delegate->max_size = max_size;
    }
}

// Update read to check size
ar_data_t* ar_file_delegate__read(ar_file_delegate_t *mut_delegate, const char *ref_path) {
    // ... existing validation ...

    // Check file size before reading
    if (mut_delegate->max_size > 0) {
        size_t file_size = ar_io__get_file_size(full_path);
        if (file_size > mut_delegate->max_size) {
            ar_log__error(mut_delegate->ref_log,
                         "File exceeds size limit: %zu > %zu",
                         file_size, mut_delegate->max_size);
            return NULL;
        }
    }

    // ... rest of read implementation ...
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

---

#### Iteration 12.2: Write fails when content exceeds size limit - PENDING REVIEW

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