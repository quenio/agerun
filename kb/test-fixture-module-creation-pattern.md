# Test Fixture Module Creation Pattern

## Learning
Test helper functions should be extracted into proper fixture modules with opaque types, clear ownership semantics, and lifecycle management functions rather than being scattered across test files.

## Importance
- Promotes code reuse across multiple test files
- Follows Parnas information hiding principles
- Provides clear ownership and resource management
- Enables proper abstraction of test infrastructure
- Reduces code duplication in tests

## Example
```c
// Header file with opaque type
typedef struct ar_executable_fixture_s ar_executable_fixture_t;

// Lifecycle management functions
ar_executable_fixture_t* ar_executable_fixture__create(void);
void ar_executable_fixture__destroy(ar_executable_fixture_t *own_fixture);

// Resource management with clear ownership
char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture);
void ar_executable_fixture__destroy_methods_dir(ar_executable_fixture_t *mut_fixture,
                                                char *own_methods_dir);

// Implementation with proper structure
struct ar_executable_fixture_s {
    char temp_build_dir[256];     /* Temporary build directory path */
    bool initialized;              /* Whether build directory has been initialized */
};

// Create function with initialization
ar_executable_fixture_t* ar_executable_fixture__create(void) {
    ar_executable_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_executable_fixture_t), 
                                                            "Executable fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    // Initialize fields
    own_fixture->temp_build_dir[0] = '\0';
    own_fixture->initialized = false;
    
    // Initialize resources
    _init_temp_build_dir(own_fixture);
    
    return own_fixture; // Ownership transferred to caller
}
```

## Generalization
Pattern for creating test fixture modules:
1. **Opaque Type**: Define struct in .c file, typedef in .h file
2. **Lifecycle Functions**: Provide create/destroy pairs
3. **Resource Management**: Use ownership prefixes (own_, mut_, ref_)
4. **Helper Functions**: Make static in .c file for internal use
5. **Documentation**: Document ownership transfer in comments
6. **Memory Tracking**: Use AR__HEAP__ macros for allocations
7. **NULL Handling**: Check parameters and handle gracefully

## Implementation
```c
// Test using the fixture
void test_executable_behavior(void) {
    // Create fixture
    ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();
    AR_ASSERT(own_fixture != NULL, "Should create fixture");
    
    // Use fixture resources
    char *own_methods_dir = ar_executable_fixture__create_methods_dir(own_fixture);
    AR_ASSERT(own_methods_dir != NULL, "Should create methods directory");
    
    // Run tests with fixture
    FILE *pipe = ar_executable_fixture__build_and_run(own_fixture, own_methods_dir);
    // ... test logic ...
    pclose(pipe);
    
    // Clean up resources
    ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir);
    
    // Destroy fixture
    ar_executable_fixture__destroy(own_fixture);
}
```

## Related Patterns
- [Opaque Types Principle](opaque-types-principle.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Test Fixture Simplification Pattern](test-fixture-simplification-pattern.md)
- [Module Development Patterns](module-development-patterns.md)
- [Test Fixture API Adaptation](test-fixture-api-adaptation.md)