# BDD Test Structure

**Note**: Examples include `ar_delegate` message queue functions (`send`, `take_message`) which are planned for implementation in TDD Cycle 6.5, not yet implemented.

## Learning
Use mandatory Given/When/Then comments in ALL tests to clearly document test intent and flow

## Importance
Makes tests self-documenting and ensures clear understanding of what is being tested and why

## Example
```c  // EXAMPLE: TDD Cycle 6.5 planned functions
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
- **When**: Describe the action being tested (including creation of inputs for the action)
- **Then**: Describe the expected result and verification
- **Cleanup**: Describe resource cleanup (destroy owned resources)
- Use these exact comment patterns in ALL tests
- **REQUIRED**: Use AR_ASSERT macros, not plain assert() (AGENTS.md requirement)
- One test per behavior (focused testing)
- Keep tests isolated and fast

### BDD Section Guidelines

**Given Section**: Context and preconditions
- Create system/module instances needed for the test
- Set up initial state
- Do NOT create the data being sent/processed - that belongs in When

**When Section**: The action and its inputs
- Create the message/data being sent (this is part of the action)
- Perform the operation being tested
- Capture results

**Then Section**: Verification
- Assert expected outcomes
- Check state changes
- Verify return values

**Cleanup Section**: Resource cleanup
- Destroy all owned resources
- **CRITICAL**: Destroy messages returned from take_message functions
  - These functions transfer ownership to the caller
  - Caller MUST destroy non-NULL returned messages to prevent leaks

### Message Ownership in Tests

When testing message queue operations:

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
static void test_delegate__take_message_returns_sent_message(void) {
    // Given a delegate with a sent message
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When sending and taking a message
    ar_data_t *own_message = ar_data__create_string("hello");  // Message creation in When
    ar_delegate__send(own_delegate, own_message);
    ar_data_t *own_received = ar_delegate__take_message(own_delegate);

    // Then it should return the message
    AR_ASSERT(own_received != NULL, "Should return message");

    // Cleanup - MUST destroy message (delegate dropped ownership)
    ar_data__destroy(own_received);  // MANDATORY: caller owns the message now
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

**Pattern**: Any function named `take_*` transfers ownership → caller must destroy

## Related Patterns
- [Function Naming State Change Convention](function-naming-state-change-convention.md) - take_ vs get_ naming
- [Message Ownership Flow](message-ownership-flow.md) - Complete ownership lifecycle
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md) - Planning test iterations
- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md) - Proper RED phases
- [AR_ASSERT for Descriptive Failures](ar-assert-descriptive-failures.md)
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)
- [Standards Over Expediency Principle](standards-over-expediency-principle.md)
- [Test Standardization Retroactive Application](test-standardization-retroactive.md)
- [Test Signal Reporting Practice](test-signal-reporting-practice.md)