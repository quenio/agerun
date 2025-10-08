# Test Isolation Shared Directory Pattern

## Learning
When tests run in a shared build directory and create persisted files (like `agerun.agency` or `agerun.methodology`), subsequent tests can load those files instead of starting clean. This creates test interdependencies where test order matters and failures in one test affect others. The solution is explicit cleanup in test setup or using unique temporary directories per test.

## Importance
Test isolation violations cause:
- **Flaky tests**: Pass or fail depending on execution order
- **Hidden bugs**: Real issues masked by leftover state
- **Debugging difficulty**: Failures don't reproduce in isolation
- **CI failures**: Different behavior between local and CI environments
- **Maintenance burden**: Changing one test breaks others

Proper isolation ensures tests are independent, reproducible, and reliable.

## Example
```c
// PROBLEM: Tests share build directory without cleanup

// Test 1: Saves agents to agerun.agency
void test_executable__saves_agents_on_shutdown(void) {
    ar_executable_fixture_t* mut_fixture = ar_executable_fixture__create();

    // Creates agents and saves them to agerun.agency
    ar_system_t* mut_system = /* ... */;
    ar_agency__save_agents(ar_system__get_agency(mut_system), "agerun.agency");

    ar_executable_fixture__destroy(mut_fixture);
    // File agerun.agency still exists in build directory!
}

// Test 2: Expects bootstrap agent creation
void test_bootstrap_agent_creation(void) {
    ar_executable_fixture_t* mut_fixture = ar_executable_fixture__create();

    // Code checks for agerun.agency and loads it
    // Finds agents from previous test instead of creating bootstrap!
    ar_system_t* mut_system = /* ... */;

    // FAILS: Expected "Creating bootstrap agent" but got "Agents loaded from disk"
    AR_ASSERT(bootstrap_created, "Should create bootstrap agent");

    ar_executable_fixture__destroy(mut_fixture);
}

// SOLUTION 1: Explicit cleanup helper in fixture

// ar_executable_fixture.c
void ar_executable_fixture__clean_persisted_files(  // EXAMPLE: Real function from actual fixture
    const ar_executable_fixture_t* ref_fixture  // Real type
) {
    if (!ref_fixture || !ref_fixture->initialized) {
        return;
    }

    // Remove methodology file
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path),
             "%s/agerun.methodology", ref_fixture->temp_build_dir);
    remove(methodology_path);

    // Remove agency file
    char agency_path[512];
    snprintf(agency_path, sizeof(agency_path),
             "%s/agerun.agency", ref_fixture->temp_build_dir);
    remove(agency_path);
}

// Test with cleanup
void test_bootstrap_agent_creation_isolated(void) {  // EXAMPLE: Hypothetical test name
    ar_executable_fixture_t* mut_fixture = ar_executable_fixture__create();  // Real function

    // Clean up any persisted files from previous tests
    ar_executable_fixture__clean_persisted_files(mut_fixture);

    // Now test starts clean
    ar_system_t* mut_system = /* ... */;

    AR_ASSERT(bootstrap_created, "Should create bootstrap agent");

    ar_executable_fixture__destroy(mut_fixture);
}

// SOLUTION 2: Unique temp directory per test
// EXAMPLE: Hypothetical pattern showing temp directory approach
// In practice, use real fixture types like ar_executable_fixture_t

// Conceptual structure for teaching purposes:
// - temp_dir: Unique directory path
// - mut_methodology: ar_methodology_t* instance
// - mut_agency: ar_agency_t* instance

void* create_test_context(void) {  // EXAMPLE: Conceptual function
    // In practice: Use ar_executable_fixture__create() or similar
    char temp_dir[512];

    // Create unique temp directory using PID and timestamp
    snprintf(temp_dir, sizeof(temp_dir),
             "/tmp/agerun_test_%d_%ld", getpid(), time(NULL));
    mkdir(temp_dir, 0755);

    // Change to temp directory
    chdir(temp_dir);

    ar_methodology_t* mut_methodology = ar_methodology__create();
    ar_agency_t* mut_agency = ar_agency__create(mut_methodology);

    // Store these for cleanup...
    return NULL;  // Placeholder
}

void destroy_test_context(void* context) {  // EXAMPLE: Conceptual function
    // In practice: Use ar_executable_fixture__destroy() or similar
    // Cleanup would include:
    // - ar_agency__destroy(mut_agency)
    // - ar_methodology__destroy(mut_methodology)
    // - Remove temp directory: system("rm -rf /tmp/agerun_test_*")
}
```

## Generalization
**Test isolation strategies**:

1. **Explicit cleanup** (simple, works for shared directories):
   - Create cleanup helper in test fixture
   - Call cleanup at start of each test
   - Remove all persisted files
   - Pros: Simple, visible in test code
   - Cons: Easy to forget, requires discipline

2. **Unique temp directories** (robust, prevents conflicts):
   - Each test gets own temp directory
   - Use PID + timestamp for uniqueness
   - Automatic cleanup on fixture destroy
   - Pros: Complete isolation, no conflicts
   - Cons: More complex setup, disk space

3. **Setup/teardown pattern** (systematic):
   - Before each test: Clean or create fresh directory
   - After each test: Remove test files
   - Pros: Consistent, hard to forget
   - Cons: Requires test framework support

**Detection of isolation issues**:
- Tests pass individually but fail when run together
- Tests fail in CI but pass locally
- Test failures disappear after `make clean`
- Different results depending on test order

## Implementation
```bash
# Detection: Find tests that create persisted files
grep -r "agerun\.\|\.yaml\|\.json" modules/*_tests.c

# For each test file creating persistence:

# 1. Add cleanup helper to fixture
# See ar_executable_fixture.c lines 165-180 for example

# 2. Call cleanup at test start
void test_that_needs_clean_state(void) {  // EXAMPLE: Hypothetical test
    ar_executable_fixture_t* mut_fixture = ar_executable_fixture__create();  // Real function
    ar_executable_fixture__clean_persisted_files(mut_fixture);  // Real cleanup function

    // Rest of test...
}

# 3. Or use unique temp directories
# See ar_test_context pattern above

# 4. Verify isolation
# Run tests multiple times in different orders
make clean && make test  # First run
make test                # Second run without clean
# Should get same results

# Run tests in reverse order
# If results differ, isolation is broken
```

**Cleanup helper pattern**:
```c
// In fixture implementation
void ar_module_fixture__clean_persisted_files(  // EXAMPLE: Generic pattern - use actual fixture name
    const ar_executable_fixture_t* ref_fixture  // Real type
) {  // EXAMPLE: Pattern for any fixture module
    if (!ref_fixture) return;

    // List all files that tests might create
    const char* files[] = {
        "agerun.agency",
        "agerun.methodology",
        "test_output.yaml",
        "temp_data.json",
        NULL
    };

    // Remove each file
    for (int i = 0; files[i] != NULL; i++) {
        char path[512];
        snprintf(path, sizeof(path),
                 "%s/%s", ref_fixture->build_dir, files[i]);
        remove(path);  // Safe if file doesn't exist
    }
}
```

## Related Patterns
- [Redundant Test Cleanup Anti-Pattern](redundant-test-cleanup-anti-pattern.md)
- [TDD Feature Completion Before Commit](tdd-feature-completion-before-commit.md)
