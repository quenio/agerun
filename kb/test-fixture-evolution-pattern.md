# Test Fixture Evolution Pattern

## Learning
Test fixtures should be designed to evolve organically as new testing needs arise. Well-designed fixtures can add helper methods and functionality without breaking existing tests.

## Importance
As test suites grow, fixtures need to support increasingly complex scenarios. Extensible fixtures reduce code duplication and make tests more maintainable. Poor fixture design leads to test brittleness and duplicated setup code.

## Example
```c
// Original fixture - basic functionality
typedef struct ar_executable_fixture_s ar_executable_fixture_t;

ar_executable_fixture_t* ar_executable_fixture__create(void);
void ar_executable_fixture__destroy(ar_executable_fixture_t *own_fixture);
char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture);
FILE* ar_executable_fixture__build_and_run(const ar_executable_fixture_t *ref_fixture, 
                                           const char *ref_methods_dir);

// Evolution: New test needs access to build directory
// Solution: Add helper method without changing existing interface
const char* ar_executable_fixture__get_build_dir(const ar_executable_fixture_t *ref_fixture);

// Implementation maintains backward compatibility
const char* ar_executable_fixture__get_build_dir(const ar_executable_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->temp_build_dir;  // Expose internal state safely
}

// New test can now access build directory for advanced scenarios
static void test_executable__continues_on_save_failure(ar_executable_fixture_t *mut_fixture) {
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
    // ... use path for testing ...
}
```

## Generalization
Fixture evolution principles:
1. **Encapsulation**: Keep internal state private, expose through methods
2. **Additive changes**: Add new methods rather than changing existing ones
3. **Backward compatibility**: Never break existing test code
4. **Single responsibility**: Each helper method does one thing well
5. **Resource management**: Maintain clear ownership patterns

Common evolution patterns:
- Add getters for internal state
- Add configuration methods for setup variations
- Add cleanup helpers for resource management
- Add validation methods for assertions

## Implementation
```c
// Pattern for extensible fixtures (using real AgeRun fixture as example)
struct ar_executable_fixture_s {
    // Core state
    char temp_build_dir[256];
    bool initialized;
    
    // Extension points for future needs
    ar_map_t *own_config;     // Configuration options
    ar_list_t *own_resources;  // Dynamic resource list
};

// Evolution pathway demonstrated with real fixture:
// Version 1: Basic create/destroy
ar_executable_fixture_t* ar_executable_fixture__create(void);
void ar_executable_fixture__destroy(ar_executable_fixture_t *own_fixture);

// Version 2: Add methods directory management
char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture);

// Version 3: Add build and run capability
FILE* ar_executable_fixture__build_and_run(const ar_executable_fixture_t *ref_fixture,
                                           const char *ref_methods_dir);

// Version 4: Add resource access (actual addition from this session)
const char* ar_executable_fixture__get_build_dir(const ar_executable_fixture_t *ref_fixture);
```

## Related Patterns
- [Test Fixture Module Creation Pattern](test-fixture-module-creation-pattern.md)
- [Dynamic Test Resource Allocation](dynamic-test-resource-allocation.md)
- [Test Fixture Simplification Pattern](test-fixture-simplification-pattern.md)