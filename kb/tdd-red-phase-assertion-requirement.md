# TDD RED Phase Assertion Requirement

## Learning
The RED phase of TDD must produce **assertion failures**, not compilation errors. This requires: (1) adding function declarations to headers first, (2) implementing stub functions that return NULL/false, (3) writing tests that compile but fail assertions.

## Importance
Proper TDD RED phase validates that tests are actually testing something and can detect failures. Compilation errors don't verify test logic works correctly. Assertion failures prove the test can catch bugs when the implementation is wrong.

## Example
```c
// WRONG RED Phase: Function doesn't exist → compilation error
// In ar_system_tests.c:
static void test_system__has_proxy_registry(void) {
    ar_system_t *mut_system = ar_system__create();
    ar_proxy_registry_t *ref_registry = ar_system__get_proxy_registry(mut_system);
    AR_ASSERT(ref_registry != NULL, "System should have a proxy registry");
    ar_system__destroy(mut_system);
}
// RESULT: Compilation fails because ar_system__get_proxy_registry() doesn't exist
// PROBLEM: We don't know if the test logic is correct

// CORRECT RED Phase: Stub exists → assertion failure

// Step 1: Add function declaration to header (ar_system.h)
ar_proxy_registry_t* ar_system__get_proxy_registry(const ar_system_t *ref_system);

// Step 2: Add stub implementation (ar_system.c)
ar_proxy_registry_t* ar_system__get_proxy_registry(const ar_system_t *ref_system) {
    return NULL;  // Stub always returns NULL
}

// Step 3: Write test (ar_system_tests.c)
static void test_system__has_proxy_registry(void) {
    // Given a system instance
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");

    // When we get the proxy registry
    ar_proxy_registry_t *ref_registry = ar_system__get_proxy_registry(mut_system);

    // Then the registry should exist
    AR_ASSERT(ref_registry != NULL, "System should have a proxy registry");

    // Clean up
    ar_system__destroy(mut_system);
}

// RESULT: Test compiles successfully
// RESULT: Test runs and assertion fails: "System should have a proxy registry"
// BENEFIT: We know the test logic is correct and can catch the bug
```

## Generalization
**Proper TDD RED Phase Workflow:**

1. **Add Function Declaration** (header file):
   ```c
   // In modules/ar_system.h
   bool ar_system__register_proxy(ar_system_t *mut_system,
                                   int64_t proxy_id,
                                   ar_proxy_t *own_proxy);
   ```

2. **Implement Stub** (implementation file):
   ```c
   // In modules/ar_system.c
   bool ar_system__register_proxy(ar_system_t *mut_system,
                                   int64_t proxy_id,
                                   ar_proxy_t *own_proxy) {
       return false;  // Stub returns failure
   }
   ```

3. **Write Test** (test file):
   ```c
   // In modules/ar_system_tests.c
   static void test_system__register_proxy(void) {
       // Given a system and proxy
       ar_system_t *mut_system = ar_system__create();
       ar_log_t *ref_log = ar_system__get_log(mut_system);
       ar_proxy_t *own_proxy = ar_proxy__create(ref_log, "test");

       // When we register the proxy
       bool result = ar_system__register_proxy(mut_system, -100, own_proxy);

       // Then registration should succeed
       AR_ASSERT(result, "Proxy registration should succeed");

       ar_system__destroy(mut_system);
   }
   // RESULT: Compiles, runs, assertion fails on "result" being false
   ```

4. **Verify RED Phase**:
   - Run tests: `make test`
   - See assertion failure message (not compilation error)
   - Confirm test can detect the bug

**Stub Return Values by Type:**
- `bool` functions → return `false`
- Pointer functions → return `NULL`
- Integer functions → return `0` or error code
- Enum functions → return error/invalid state

## Implementation
```bash
# RED Phase execution order (from TDD Cycle 4.5)

# 1. Add function declarations to header
# modules/ar_system.h: Add ar_system__get_proxy_registry() signature

# 2. Add stub implementations
# modules/ar_system.c: Return NULL for getter, false for register

# 3. Write tests
# modules/ar_system_tests.c: Add test functions

# 4. Build and run tests
make test

# Expected output:
# test_system__has_proxy_registry...
# Assertion failed: System should have a proxy registry
# test_system__register_proxy...
# Assertion failed: Proxy registration should succeed

# 5. Verify RED phase complete
# - Code compiles successfully ✓
# - Tests run ✓
# - Assertions fail with descriptive messages ✓
# - Ready for GREEN phase implementation
```

## Why Compilation Errors Are Wrong
Compilation errors in RED phase have several problems:

1. **No test validation**: Can't verify test logic is correct
2. **False confidence**: Test might pass once function exists but not actually test anything
3. **Incomplete coverage**: Might not be testing all the requirements
4. **Poor assertions**: Might have weak assertions that don't catch bugs

Example of why this matters:
```c
// Test written against non-existent function
static void test_agent__send_message(void) {  // EXAMPLE: Hypothetical test
    ar_agent_t *mut_agent = ar_agent__create(NULL, "test");
    ar_data_t *own_msg = ar_data__create_string("hello");

    // Test compiles once function exists
    ar_agent__send(mut_agent, own_msg);  // Real function used

    // But wait - no assertions! Test doesn't verify anything!
    ar_agent__destroy(mut_agent);
}

// If we had proper RED phase with stubs, we would notice:
// "There are no assertions - this test doesn't verify behavior"
```

## Compilation-Based TDD Exception
Note: [Compilation-Based TDD Approach](compilation-based-tdd-approach.md) is valid for **infrastructure removal** (removing parameters, global constants, fields), but normal feature development should use assertion-based RED phase.

## Related Patterns
- [Compilation-Based TDD Approach](compilation-based-tdd-approach.md) - Exception for removal work
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md) - Complete TDD cycle
- [BDD Test Structure](bdd-test-structure.md) - Test organization
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md) - Writing good assertions
- [AR_ASSERT for Descriptive Failures](ar-assert-descriptive-failures.md) - Clear error messages
