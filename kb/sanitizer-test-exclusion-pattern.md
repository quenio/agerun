# Sanitizer Test Exclusion Pattern

## Learning
Tests that use dlsym for function interception (e.g., malloc/free hooking) conflict with sanitizer instrumentation and cause segmentation faults. These tests must be excluded from sanitizer builds while still running in regular builds.

## Importance
Sanitizers instrument code at compile and runtime, which conflicts with dlsym-based function interception. This causes crashes in CI environments where sanitizers are strictly enforced, even though the tests may work locally.

## Example
```makefile
# Test source files
TEST_SRC = $(wildcard modules/*_tests.c)

# Filtered test sources for sanitizers (exclude *_dlsym_tests.c)
SANITIZER_TEST_SRC = $(filter-out %_dlsym_tests.c,$(TEST_SRC))
SANITIZER_METHOD_TEST_SRC = $(filter-out %_dlsym_tests.c,$(METHOD_TEST_SRC))

# Use filtered lists for sanitizer targets
sanitize-tests:
	$(MAKE) $(SANITIZE_TESTS_TEST_BIN) # Uses SANITIZER_TEST_SRC
	@cd $(SANITIZE_TESTS_DIR) && for test in $(SANITIZER_ALL_TEST_BIN_NAMES); do \
		./$$test; \
	done
```

Example test that needs exclusion:
```c
/**
 * @file ar_instruction_evaluator_dlsym_tests.c
 * @brief Tests using dlsym function interception
 * 
 * Note: This test is excluded from sanitizer builds because dlsym interception
 * conflicts with sanitizer instrumentation.
 */

// Function interception that conflicts with sanitizers
static void* (*real_malloc)(size_t) = NULL;
static void (*real_free)(void*) = NULL;

static void intercept_functions(void) {
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
}
```

## Generalization
1. **Naming Convention**: Use `*_dlsym_tests.c` suffix for tests using dlsym interception
2. **Makefile Filtering**: Create filtered test lists using `$(filter-out %_dlsym_tests.c,$(TEST_SRC))`
3. **Documentation**: Add clear comments explaining why the test is excluded from sanitizers
4. **Verification**: Ensure excluded tests still run in regular builds

## Implementation
```bash
# Verify test exclusion works correctly
make run-tests 2>&1 | grep -c "dlsym_tests"  # Should show count > 0
make sanitize-tests 2>&1 | grep -c "dlsym_tests"  # Should show 0
make tsan-tests 2>&1 | grep -c "dlsym_tests"  # Should show 0

# Check logs for confirmation
grep "dlsym_tests" logs/run-tests.log  # Should find test execution
grep "dlsym_tests" logs/sanitize-tests.log  # Should find nothing
```

## Related Patterns
- [DLSym Malloc Retry Logic Pattern](dlsym-malloc-retry-logic-pattern.md) - Handling retry logic when testing malloc failures with dlsym
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [DLSym Test Interception Technique](dlsym-test-interception-technique.md)