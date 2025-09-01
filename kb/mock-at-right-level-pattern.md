# Mock at Right Level Pattern

## Learning
When testing with dlsym interception, mock at the function level rather than system calls. Intercepting low-level calls like malloc creates unnecessary complexity due to framework retry logic and implementation details. Mock the highest-level function that provides the behavior you need to test.

## Importance
Testing at the wrong abstraction level leads to:
- Complex test setup with loops and conditions
- Fighting against framework retry mechanisms
- Brittle tests that break with implementation changes
- Difficulty understanding test intent
- Maintenance burden from complex mocking logic

## Example
```c
// BAD: Mocking malloc to test parser creation failure
void* malloc(size_t size) {
    static int fail_count = 0;
    if (++fail_count == 3) {  // Complex retry logic
        return NULL;
    }
    return real_malloc(size);
}

// GOOD: Mock the parser creation function directly
ar_assignment_instruction_parser_t* ar_assignment_instruction_parser__create(ar_log_t* log) {
    if (should_fail_parser_create) {
        if (log) {
            ar_log__error(log, "Failed to allocate memory for parser");
        }
        return NULL;
    }
    return (ar_assignment_instruction_parser_t*)0x1000;  // Fake pointer
}
```

## Generalization
Choose the mocking level based on what you're testing:
- Testing error handling? Mock the function that can fail
- Testing cleanup? Mock creation/destruction pairs
- Testing integration? Mock at component boundaries
- Never mock system calls unless testing system call handling specifically

## Implementation
```c
// Pattern for clean function mocking
static bool should_fail_create = false;

// Mock the specific function you're testing
ar_data_t* ar_data__create_map(void) {
    if (should_fail_create) {
        return NULL;
    }
    return (ar_data_t*)0x1000;  // Return fake pointer
}

// Test uses simple flag control
void test_handles_creation_failure(void) {
    should_fail_create = true;
    ar_data_t* result = some_function_that_creates_map();
    AR_ASSERT(result == NULL, "Should handle creation failure");
}
```

## Related Patterns
- [DLSym Test Interception Technique](dlsym-test-interception-technique.md)
- [Test Complexity as Code Smell](test-complexity-as-code-smell.md)
- [Test Effectiveness Verification](test-effectiveness-verification.md)