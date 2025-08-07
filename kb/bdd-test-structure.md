# BDD Test Structure

## Learning
Use mandatory Given/When/Then comments in ALL tests to clearly document test intent and flow

## Importance
Makes tests self-documenting and ensures clear understanding of what is being tested and why

## Example
```c
#include "ar_assert.h"

static void test_data_create_map(void) {
    // Given a call to create a new map
    ar_data_t *own_map = ar_data__create_map();
    
    // When checking the created map
    ar_data_type_t type = ar_data__get_type(own_map);
    
    // Then it should be a map type
    AR_ASSERT(type == AR_DATA_TYPE__MAP, "Created data should be of map type");
    
    // Cleanup
    ar_data__destroy(own_map);
}
```

## Generalization
Structure tests to clearly communicate setup, action, and verification phases

## Implementation
- **Given**: Describe the test setup and preconditions
- **When**: Describe the action being tested
- **Then**: Describe the expected result and verification
- Use these exact comment patterns in ALL tests
- One test per behavior (focused testing)
- Keep tests isolated and fast

## Related Patterns
- [AR_ASSERT for Descriptive Failures](ar-assert-descriptive-failures.md)
- [Test Standardization Retroactive Application](test-standardization-retroactive.md)
- [Test Signal Reporting Practice](test-signal-reporting-practice.md)