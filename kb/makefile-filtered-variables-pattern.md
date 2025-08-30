# Makefile Filtered Variables Pattern

## Learning
Make's `filter-out` function enables creating filtered variable sets for conditional compilation and execution, allowing different build targets to use customized file lists without duplication.

## Importance
This pattern eliminates the need to maintain separate file lists for each build configuration, reducing maintenance burden and preventing inconsistencies between targets.

## Example
```makefile
# Base file lists
C_SRC = $(wildcard modules/*.c)
TEST_SRC = $(wildcard modules/*_tests.c)
METHOD_TEST_SRC = $(wildcard methods/*_tests.c)

# Filtered lists for specific purposes
# Exclude test files from main source
SRC_NO_TESTS = $(filter-out %_tests.c,$(C_SRC))

# Exclude dlsym tests from sanitizer builds
SANITIZER_TEST_SRC = $(filter-out %_dlsym_tests.c,$(TEST_SRC))
SANITIZER_METHOD_TEST_SRC = $(filter-out %_dlsym_tests.c,$(METHOD_TEST_SRC))

# Create corresponding object and binary lists
SANITIZER_TEST_OBJ = $(patsubst modules/%.c,$(SANITIZER_DIR)/obj/%.o,$(SANITIZER_TEST_SRC))
SANITIZER_TEST_BIN = $(patsubst modules/%_tests.c,$(SANITIZER_DIR)/%_tests,$(SANITIZER_TEST_SRC))

# Use in targets
sanitize-tests: $(SANITIZER_TEST_BIN)  # Only builds filtered tests
run-tests: $(TEST_BIN)  # Builds all tests
```

## Generalization
1. **Pattern Functions**:
   - `$(filter pattern,text)` - Keep only matching items
   - `$(filter-out pattern,text)` - Remove matching items
   - `$(patsubst pattern,replacement,text)` - Pattern substitution

2. **Common Use Cases**:
   - Exclude files by suffix: `$(filter-out %_suffix.c,$(FILES))`
   - Include only certain types: `$(filter %.c %.h,$(ALL_FILES))`
   - Transform paths: `$(patsubst src/%,obj/%,$(SOURCES))`

3. **Chaining Filters**:
   ```makefile
   # Multiple exclusions
   FILTERED = $(filter-out %_test.c,$(filter-out %_bench.c,$(ALL_C)))
   ```

## Implementation
```makefile
# Debug filtered variables
print-vars:
	@echo "All tests: $(words $(TEST_SRC)) files"
	@echo "Sanitizer tests: $(words $(SANITIZER_TEST_SRC)) files"
	@echo "Excluded: $(filter %_dlsym_tests.c,$(TEST_SRC))"

# Verify filtering works
check-filtering:
	@if [ $(words $(SANITIZER_TEST_SRC)) -ge $(words $(TEST_SRC)) ]; then \
		echo "ERROR: Filtering not working"; exit 1; \
	fi
```

## Related Patterns
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Sanitizer Test Exclusion Pattern](sanitizer-test-exclusion-pattern.md)
- [Generic Make Targets Pattern](generic-make-targets-pattern.md)