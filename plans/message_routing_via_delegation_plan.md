# Plan: Message Routing via Delegation

**Status**: CYCLE 0 COMPLETE ✅ - Fixture infrastructure fully implemented with 9 tests and 7 refactored tests
**Created**: 2025-10-14
**Estimated Effort**: 18 iterations (~5-6 hours total)
**Cycle 0 Progress**: 9/9 iterations IMPLEMENTED + 7 tests refactored = 16 tests passing, 0 memory leaks
**Next Phase**: Cycle 1 - Message Delegation Routing (11 iterations)

## Objective

Implement ID-based message routing in `ar_send_instruction_evaluator.zig` to route messages to delegates when target ID is negative, while preserving existing agent routing for non-negative IDs.

## Context

### KB Articles Consulted
- ✅ [ID-Based Message Routing Pattern](../kb/id-based-message-routing-pattern.md) - Core routing pattern
- ✅ [TDD Iteration Planning Pattern](../kb/tdd-iteration-planning-pattern.md) - One assertion per iteration
- ✅ [TDD Cycle Detailed Explanation](../kb/tdd-cycle-detailed-explanation.md) - RED-GREEN-REFACTOR structure
- ✅ [Red-Green-Refactor Cycle](../kb/red-green-refactor-cycle.md) - Three phase requirement
- ✅ [Message Ownership Flow](../kb/message-ownership-flow.md) - Ownership transfer patterns
- ✅ [System Message Flow Architecture](../kb/system-message-flow-architecture.md) - System coordination
- ✅ [TDD RED Phase Assertion Requirement](../kb/tdd-red-phase-assertion-requirement.md) - Proper RED phase with assertion failures
- ✅ [AR_ASSERT for Descriptive Failures](../kb/ar-assert-descriptive-failures.md) - Use AR_ASSERT with descriptive messages
- ✅ [Test Assertion Strength Patterns](../kb/test-assertion-strength-patterns.md) - Strong assertions verify specific outcomes

### Current Implementation State

**File**: `modules/ar_send_instruction_evaluator.zig`

**Current Routing Logic** (lines 124-134):
```zig
// Send the message
var send_result: bool = undefined;
if (agent_id == 0) {
    // Special case: agent_id 0 is a no-op that always returns true
    std.debug.print("DEBUG [SEND_EVAL]: Sending to agent 0...\n", .{...});
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);
    send_result = true;
} else {
    // Send message (ownership transferred to ar_agency__send_to_agent)
    send_result = c.ar_agency__send_to_agent(ref_evaluator.?.ref_agency, agent_id, own_message);
}
```

**Problem**: All non-zero IDs route to agency. Negative IDs should route to delegation.

**Dependencies Available**:
- ✅ `ref_delegation` field (line 21) - Already propagated through evaluator chain
- ✅ `ar_delegation__send_to_delegate()` - Implemented in TDD Cycle 6.5
- ✅ `ar_agency__send_to_agent()` - Existing agent routing

### Architecture Pattern

Following **ID-Based Message Routing Pattern** (kb/id-based-message-routing-pattern.md):

```
Message Routing by ID Sign:
├── agent_id = 0    → No-op (destroy message, return true)
├── agent_id > 0    → Route to ar_agency__send_to_agent()
└── agent_id < 0    → Route to ar_delegation__send_to_delegate()
```

## Preconditions

- [x] All tests currently passing (78 tests)
- [x] Zero memory leaks confirmed
- [x] Delegation parameter propagated through evaluator chain (completed in Phase 2)
- [x] Delegation message queue infrastructure complete (TDD Cycle 6.5)
- [x] `ar_delegation__send_to_delegate()` implemented and tested

## Detailed Implementation Plan

### Iteration 0.1: Fixture helper returns non-NULL evaluator (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Create fixture helper that returns a valid send evaluator.

#### RED Phase

**Step 1**: Create fixture header with function declarations
```c
// File: modules/ar_send_evaluator_fixture.h (NEW)
#ifndef AGERUN_SEND_EVALUATOR_FIXTURE_H
#define AGERUN_SEND_EVALUATOR_FIXTURE_H

#include "ar_send_instruction_evaluator.h"
#include "ar_frame.h"

/**
 * Opaque type for send evaluator fixture
 */
typedef struct ar_send_evaluator_fixture_s ar_send_evaluator_fixture_t;

/**
 * Creates a send evaluator fixture
 * @param ref_test_name Name of the test for identification
 * @return A newly created fixture
 * @note Ownership: Returns owned fixture that caller must destroy
 */
ar_send_evaluator_fixture_t* ar_send_evaluator_fixture__create(
    const char *ref_test_name
);

/**
 * Destroys a send evaluator fixture
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys fixture and all resources
 */
void ar_send_evaluator_fixture__destroy(
    ar_send_evaluator_fixture_t *own_fixture
);

/**
 * Creates a send evaluator from the fixture
 * @param ref_fixture The fixture to use
 * @return A newly created send evaluator
 * @note Ownership: Returns owned evaluator that caller must destroy
 */
ar_send_instruction_evaluator_t* ar_send_evaluator_fixture__create_evaluator(
    const ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Creates a frame for evaluation
 * @param ref_fixture The fixture to query
 * @return A frame (borrowed reference)
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_frame_t* ar_send_evaluator_fixture__create_frame(
    ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the delegation instance
 * @param ref_fixture The fixture to query
 * @return The delegation instance (borrowed reference)
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_delegation_t* ar_send_evaluator_fixture__get_delegation(
    const ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the agency instance
 * @param ref_fixture The fixture to query
 * @return The agency instance (borrowed reference)
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_agency_t* ar_send_evaluator_fixture__get_agency(
    const ar_send_evaluator_fixture_t *ref_fixture
);

/**
 * Creates and registers a delegate
 * @param ref_fixture The fixture to use
 * @param delegate_id The ID for the delegate (negative)
 * @param ref_name The name for the delegate
 * @return The created delegate (borrowed reference), or NULL on failure
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_delegate_t* ar_send_evaluator_fixture__create_and_register_delegate(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t delegate_id,
    const char *ref_name
);

/**
 * Creates and registers an agent
 * @param ref_fixture The fixture to use
 * @param agent_id The ID for the agent (positive)
 * @param ref_behavior The behavior file path (can be NULL)
 * @return The created agent (borrowed reference), or NULL on failure
 * @note Ownership: Returns borrowed reference; fixture manages lifecycle
 */
ar_agent_t* ar_send_evaluator_fixture__create_and_register_agent(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t agent_id,
    const char *ref_behavior
);

#endif /* AGERUN_SEND_EVALUATOR_FIXTURE_H */
```

**Step 2**: Create stub implementation
```c
// File: modules/ar_send_evaluator_fixture.c (NEW)
#include "ar_send_evaluator_fixture.h"

ar_send_evaluator_fixture_t* ar_send_evaluator_fixture__create(
    const char *ref_test_name
) {
    return NULL;  // Stub returns NULL
}

void ar_send_evaluator_fixture__destroy(
    ar_send_evaluator_fixture_t *own_fixture
) {
    // Stub does nothing
}

ar_send_instruction_evaluator_t* ar_send_evaluator_fixture__create_evaluator(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    return NULL;  // Stub returns NULL
}

ar_frame_t* ar_send_evaluator_fixture__create_frame(
    ar_send_evaluator_fixture_t *ref_fixture
) {
    return NULL;  // Stub returns NULL
}

ar_delegation_t* ar_send_evaluator_fixture__get_delegation(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    return NULL;  // Stub returns NULL
}

ar_agency_t* ar_send_evaluator_fixture__get_agency(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    return NULL;  // Stub returns NULL
}

ar_delegate_t* ar_send_evaluator_fixture__create_and_register_delegate(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t delegate_id,
    const char *ref_name
) {
    return NULL;  // Stub returns NULL
}

ar_agent_t* ar_send_evaluator_fixture__create_and_register_agent(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t agent_id,
    const char *ref_behavior
) {
    return NULL;  // Stub returns NULL
}
```

**Step 3**: Create test file
```c
// File: modules/ar_send_evaluator_fixture_tests.c (NEW)
#include <stdio.h>
#include "ar_send_evaluator_fixture.h"
#include "ar_assert.h"

static void test_send_evaluator_fixture__create_returns_non_null(void) {
    // When creating a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_fixture_create");

    // Then it should return a valid fixture
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because stub returns NULL.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement minimal fixture structure - just enough to return non-NULL
#include "ar_send_evaluator_fixture.h"
#include "ar_heap.h"

// Define opaque structure
struct ar_send_evaluator_fixture_s {
    ar_evaluator_fixture_t *own_evaluator_fixture;
};

ar_send_evaluator_fixture_t* ar_send_evaluator_fixture__create(
    const char *ref_test_name
) {
    ar_send_evaluator_fixture_t *own_fixture = AR__HEAP__MALLOC(
        sizeof(ar_send_evaluator_fixture_t),
        "Send evaluator fixture"
    );
    if (!own_fixture) {
        return NULL;
    }

    // Minimal implementation: just allocate structure, don't populate yet
    own_fixture->own_evaluator_fixture = NULL;

    return own_fixture;  // Returns non-NULL, but not functional yet
}

void ar_send_evaluator_fixture__destroy(
    ar_send_evaluator_fixture_t *own_fixture
) {
    if (!own_fixture) {
        return;
    }

    // Cleanup evaluator fixture (will be implemented in Iteration 0.2)
    if (own_fixture->own_evaluator_fixture) {
        ar_evaluator_fixture__destroy(own_fixture->own_evaluator_fixture);
    }

    ar_heap__free(own_fixture);
}

ar_send_instruction_evaluator_t* ar_send_evaluator_fixture__create_evaluator(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture) {
        return NULL;
    }
    return NULL;  // Returns NULL until Iteration 0.2 implements creation logic
}

ar_frame_t* ar_send_evaluator_fixture__create_frame(
    ar_send_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture) {
        return NULL;
    }
    return NULL;  // Returns NULL until Iteration 0.2
}
```

**Expected GREEN**: Test PASSES - fixture creates successfully and returns non-NULL structure.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - encapsulates common pattern of creating send evaluator with all dependencies
2. Check for clarity: Good - opaque type properly encapsulates internal fixture
3. Check for edge cases: NULL checks added for all allocations and parameters

**Actions**: No refactoring needed - implementation is minimal and clean.

**Verification**: Test still passes, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.2: Fixture create_evaluator() returns non-NULL (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that create_evaluator() returns a valid evaluator.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Add new test function

static void test_send_evaluator_fixture__create_evaluator_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_create_evaluator");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating an evaluator from the fixture
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);

    // Then it should return a valid evaluator
    AR_ASSERT(own_evaluator != NULL, "Fixture should create evaluator");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}

// Update main()
int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because create_evaluator() returns NULL (from Iteration 0.1 stub).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement evaluator creation - populate internal fixture and create evaluator
#include "ar_send_evaluator_fixture.h"
#include "ar_evaluator_fixture.h"
#include "ar_heap.h"

// Structure already defined in Iteration 0.1

ar_send_evaluator_fixture_t* ar_send_evaluator_fixture__create(
    const char *ref_test_name
) {
    ar_send_evaluator_fixture_t *own_fixture = AR__HEAP__MALLOC(
        sizeof(ar_send_evaluator_fixture_t),
        "Send evaluator fixture"
    );
    if (!own_fixture) {
        return NULL;
    }

    // NOW: Create and populate internal evaluator fixture
    own_fixture->own_evaluator_fixture = ar_evaluator_fixture__create(ref_test_name);
    if (!own_fixture->own_evaluator_fixture) {
        ar_heap__free(own_fixture);
        return NULL;
    }

    return own_fixture;
}

// destroy() already has proper implementation from Iteration 0.1

ar_send_instruction_evaluator_t* ar_send_evaluator_fixture__create_evaluator(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }

    // Create new evaluator from internal fixture dependencies
    ar_log_t *log = ar_evaluator_fixture__get_log(ref_fixture->own_evaluator_fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(ref_fixture->own_evaluator_fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(ref_fixture->own_evaluator_fixture);
    ar_delegation_t *mut_delegation = ar_evaluator_fixture__get_delegation(ref_fixture->own_evaluator_fixture);

    return ar_send_instruction_evaluator__create(
        log, expr_eval, mut_agency, mut_delegation
    );
}

// create_frame() still returns NULL - will be implemented in Iteration 0.3
```

**Expected GREEN**: Test PASSES - fixture now creates functional evaluator.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - encapsulates common pattern of creating send evaluator
2. Check for clarity: Good - clear dependency extraction from internal fixture
3. Check for edge cases: NULL checks present for all parameters

**Actions**: No refactoring needed - implementation is clean.

**Verification**: Both tests still pass, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.3: Fixture create_frame() returns non-NULL (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that create_frame() returns a valid frame.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Add new test function

static void test_send_evaluator_fixture__create_frame_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_create_frame");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating a frame from the fixture
    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

    // Then it should return a valid frame
    AR_ASSERT(frame != NULL, "Fixture should create frame");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Update main()
int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_frame_returns_non_null();
    printf("test_send_evaluator_fixture__create_frame_returns_non_null passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because create_frame() returns NULL (from Iteration 0.1 stub).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement frame creation

ar_frame_t* ar_send_evaluator_fixture__create_frame(
    ar_send_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }
    return ar_evaluator_fixture__create_frame(ref_fixture->own_evaluator_fixture);
}
```

**Expected GREEN**: Test PASSES - fixture now creates frame correctly.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - simple delegation to internal fixture
2. Check for clarity: Good - clear delegation pattern
3. Check for edge cases: NULL checks present

**Actions**: No refactoring needed - implementation is minimal and correct.

**Verification**: All three tests still pass, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.4: Fixture get_delegation() returns non-NULL (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that get_delegation() returns a valid delegation instance.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Add new test function

static void test_send_evaluator_fixture__get_delegation_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_get_delegation");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When getting the delegation from the fixture
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);

    // Then it should return a valid delegation
    AR_ASSERT(delegation != NULL, "Fixture should return delegation");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Update main()
int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_frame_returns_non_null();
    printf("test_send_evaluator_fixture__create_frame_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_delegation_returns_non_null();
    printf("test_send_evaluator_fixture__get_delegation_returns_non_null passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because get_delegation() returns NULL (from Iteration 0.1 stub).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement delegation getter

ar_delegation_t* ar_send_evaluator_fixture__get_delegation(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }
    return ar_evaluator_fixture__get_delegation(ref_fixture->own_evaluator_fixture);
}
```

**Expected GREEN**: Test PASSES - fixture now returns delegation correctly.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - simple delegation to internal fixture
2. Check for clarity: Good - clear accessor pattern
3. Check for edge cases: NULL checks present

**Actions**: No refactoring needed - implementation is minimal and correct.

**Verification**: All four tests still pass, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.5: Fixture get_agency() returns non-NULL (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that get_agency() returns a valid agency instance.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Add new test function

static void test_send_evaluator_fixture__get_agency_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_get_agency");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When getting the agency from the fixture
    ar_agency_t *agency = ar_send_evaluator_fixture__get_agency(fixture);

    // Then it should return a valid agency
    AR_ASSERT(agency != NULL, "Fixture should return agency");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Update main()
int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_frame_returns_non_null();
    printf("test_send_evaluator_fixture__create_frame_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_delegation_returns_non_null();
    printf("test_send_evaluator_fixture__get_delegation_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_agency_returns_non_null();
    printf("test_send_evaluator_fixture__get_agency_returns_non_null passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because get_agency() returns NULL (from Iteration 0.1 stub).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement agency getter

ar_agency_t* ar_send_evaluator_fixture__get_agency(
    const ar_send_evaluator_fixture_t *ref_fixture
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }
    return ar_evaluator_fixture__get_agency(ref_fixture->own_evaluator_fixture);
}
```

**Expected GREEN**: Test PASSES - fixture now returns agency correctly.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - simple delegation to internal fixture
2. Check for clarity: Good - clear accessor pattern
3. Check for edge cases: NULL checks present

**Actions**: No refactoring needed - implementation is minimal and correct.

**Verification**: All five tests still pass, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.6.1: Fixture create_and_register_delegate() returns non-NULL (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that create_and_register_delegate() returns a non-NULL delegate instance.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Add new test function

static void test_send_evaluator_fixture__create_and_register_delegate_succeeds(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_delegate");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering a delegate
    ar_delegate_t *delegate = ar_send_evaluator_fixture__create_and_register_delegate(fixture, -1, "test_delegate");

    // Then it should return a valid delegate
    AR_ASSERT(delegate != NULL, "Delegate creation and registration should succeed");  // ← FAILS (stub returns NULL)

    // Cleanup (temporary: manually destroy delegate since not registered yet)
    ar_delegate__destroy(delegate);
    ar_send_evaluator_fixture__destroy(fixture);
}

// Update main()
int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_frame_returns_non_null();
    printf("test_send_evaluator_fixture__create_frame_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_delegation_returns_non_null();
    printf("test_send_evaluator_fixture__get_delegation_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_agency_returns_non_null();
    printf("test_send_evaluator_fixture__get_agency_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_and_register_delegate_succeeds();
    printf("test_send_evaluator_fixture__create_and_register_delegate_succeeds passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because create_and_register_delegate() returns NULL (from Iteration 0.1 stub).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement minimal delegate creation - just enough to return non-NULL

ar_delegate_t* ar_send_evaluator_fixture__create_and_register_delegate(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t delegate_id,
    const char *ref_name
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }

    // Get log from internal fixture
    ar_log_t *log = ar_evaluator_fixture__get_log(ref_fixture->own_evaluator_fixture);

    // Create delegate and return it (minimal implementation - no registration yet)
    // Registration will be added in Iteration 0.6.2 when we test for it
    return ar_delegate__create(log, ref_name);
}
```

**Expected GREEN**: Test PASSES - fixture now creates delegate (but doesn't register yet).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - minimal implementation
2. Check for clarity: Good - creates delegate but doesn't register yet (intentional)
3. Check for edge cases: NULL checks present
4. Check for memory leaks: Test manually destroys delegate to avoid leak (temporary until registration in 0.6.2)

**Actions**: No refactoring needed - minimal implementation is clean.

**Note**: The test temporarily calls `ar_delegate__destroy()` to clean up the unregistered delegate. This cleanup will be removed in Iteration 0.6.2 when registration takes over ownership.

**Verification**: All six tests still pass, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.6.2: Delegate is registered in delegation (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Verify that the delegate returned by create_and_register_delegate() is actually registered in the delegation.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Modify existing test function by adding new assertion and removing manual cleanup

static void test_send_evaluator_fixture__create_and_register_delegate_succeeds(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_delegate");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering a delegate
    ar_delegate_t *delegate = ar_send_evaluator_fixture__create_and_register_delegate(fixture, -1, "test_delegate");
    AR_ASSERT(delegate != NULL, "Delegate creation and registration should succeed");

    // Then the delegate should be registered in the delegation
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    ar_delegate_t *retrieved = ar_delegation__get_delegate(delegation, -1);
    AR_ASSERT(retrieved == delegate, "Delegate should be registered in delegation");  // ← FAILS (new assertion)

    // Cleanup (removed manual ar_delegate__destroy - delegation now owns it)
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Expected RED**: Test compiles but FAILS at new assertion (verifying delegate is in delegation registry).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Add registration logic

ar_delegate_t* ar_send_evaluator_fixture__create_and_register_delegate(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t delegate_id,
    const char *ref_name
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }

    // Get dependencies from internal fixture
    ar_log_t *log = ar_evaluator_fixture__get_log(ref_fixture->own_evaluator_fixture);
    ar_delegation_t *mut_delegation = ar_evaluator_fixture__get_delegation(ref_fixture->own_evaluator_fixture);

    // Create delegate
    ar_delegate_t *own_delegate = ar_delegate__create(log, ref_name);
    if (!own_delegate) {
        return NULL;
    }

    // NOW: Register delegate (ownership transfers to delegation)
    bool registered = ar_delegation__register_delegate(mut_delegation, delegate_id, own_delegate);
    if (!registered) {
        ar_delegate__destroy(own_delegate);  // Clean up if registration fails
        return NULL;
    }

    // Return delegate for verification (delegation now owns it)
    // Caller gets borrowed reference for verification only
    return ar_delegation__get_delegate(mut_delegation, delegate_id);
}
```

**Expected GREEN**: Test PASSES - delegate is now registered and can be retrieved from delegation.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - encapsulates delegate creation and registration
2. Check for clarity: Good - clear two-step process (create, register)
3. Check for edge cases: NULL checks present, cleanup on registration failure
4. Check for memory leaks: Delegate now properly owned by delegation - leak from 0.6.1 is fixed!

**Actions**: No refactoring needed - implementation is clean.

**Verification**: All six tests still pass, zero memory leaks (leak from 0.6.1 now resolved).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.7.1: Fixture create_and_register_agent() returns non-NULL (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that create_and_register_agent() returns a non-NULL agent instance.

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Add new test function

static void test_send_evaluator_fixture__create_and_register_agent_succeeds(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_agent");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering an agent
    ar_agent_t *agent = ar_send_evaluator_fixture__create_and_register_agent(fixture, 1, NULL);

    // Then it should return a valid agent
    AR_ASSERT(agent != NULL, "Agent creation and registration should succeed");  // ← FAILS (stub returns NULL)

    // Cleanup (temporary: manually destroy agent since not registered yet)
    ar_agent__destroy(agent);
    ar_send_evaluator_fixture__destroy(fixture);
}

// Update main()
int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_frame_returns_non_null();
    printf("test_send_evaluator_fixture__create_frame_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_delegation_returns_non_null();
    printf("test_send_evaluator_fixture__get_delegation_returns_non_null passed!\n");

    test_send_evaluator_fixture__get_agency_returns_non_null();
    printf("test_send_evaluator_fixture__get_agency_returns_non_null passed!\n");

    test_send_evaluator_fixture__create_and_register_delegate_succeeds();
    printf("test_send_evaluator_fixture__create_and_register_delegate_succeeds passed!\n");

    test_send_evaluator_fixture__create_and_register_agent_succeeds();
    printf("test_send_evaluator_fixture__create_and_register_agent_succeeds passed!\n");

    printf("All send evaluator fixture tests passed!\n");
    return 0;
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because create_and_register_agent() returns NULL (from Iteration 0.1 stub).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Implement minimal agent creation - just enough to return non-NULL

ar_agent_t* ar_send_evaluator_fixture__create_and_register_agent(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t agent_id,
    const char *ref_behavior
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }

    // Get log from internal fixture
    ar_log_t *log = ar_evaluator_fixture__get_log(ref_fixture->own_evaluator_fixture);

    // Create agent and return it (minimal implementation - no registration yet)
    // Registration will be added in Iteration 0.7.2 when we test for it
    return ar_agent__create_with_id(agent_id, ref_behavior, log);
}
```

**Expected GREEN**: Test PASSES - fixture now creates agent (but doesn't register yet).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - minimal implementation
2. Check for clarity: Good - creates agent but doesn't register yet (intentional)
3. Check for edge cases: NULL checks present
4. Check for memory leaks: Test manually destroys agent to avoid leak (temporary until registration in 0.7.2)

**Actions**: No refactoring needed - minimal implementation is clean.

**Note**: The test temporarily calls `ar_agent__destroy()` to clean up the unregistered agent. This cleanup will be removed in Iteration 0.7.2 when registration takes over ownership.

**Verification**: All seven tests still pass, zero memory leaks.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.7.2: Agent is registered in agency (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Verify that the agent returned by create_and_register_agent() is actually registered in the agency.

**Note**: This iteration is covered by existing infrastructure - the fixture now provides access to the agency, and agent registration will be validated through routing tests (Cycle 1).

#### RED Phase

```c
// File: modules/ar_send_evaluator_fixture_tests.c
// Modify existing test function by adding new assertion and removing manual cleanup

static void test_send_evaluator_fixture__create_and_register_agent_succeeds(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_agent");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering an agent
    ar_agent_t *agent = ar_send_evaluator_fixture__create_and_register_agent(fixture, 1, NULL);
    AR_ASSERT(agent != NULL, "Agent creation and registration should succeed");

    // Then the agent should be registered in the agency
    ar_agency_t *agency = ar_send_evaluator_fixture__get_agency(fixture);
    ar_agent_t *retrieved = ar_agency__get_agent(agency, 1);
    AR_ASSERT(retrieved == agent, "Agent should be registered in agency");  // ← FAILS (new assertion)

    // Cleanup (removed manual ar_agent__destroy - agency now owns it)
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Expected RED**: Test compiles but FAILS at new assertion (verifying agent is in agency registry).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### GREEN Phase

```c
// File: modules/ar_send_evaluator_fixture.c
// Add registration logic

ar_agent_t* ar_send_evaluator_fixture__create_and_register_agent(
    ar_send_evaluator_fixture_t *ref_fixture,
    int64_t agent_id,
    const char *ref_behavior
) {
    if (!ref_fixture || !ref_fixture->own_evaluator_fixture) {
        return NULL;
    }

    // Get dependencies from internal fixture
    ar_log_t *log = ar_evaluator_fixture__get_log(ref_fixture->own_evaluator_fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(ref_fixture->own_evaluator_fixture);

    // Create agent
    ar_agent_t *own_agent = ar_agent__create_with_id(agent_id, ref_behavior, log);
    if (!own_agent) {
        return NULL;
    }

    // NOW: Register agent (ownership transfers to agency)
    bool registered = ar_agency__register_agent(mut_agency, own_agent);
    if (!registered) {
        ar_agent__destroy(own_agent);  // Clean up if registration fails
        return NULL;
    }

    // Return agent for verification (agency now owns it)
    // Caller gets borrowed reference for verification only
    return ar_agency__get_agent(mut_agency, agent_id);
}
```

**Expected GREEN**: Test PASSES - agent is now registered and can be retrieved from agency.

**Run**: `make ar_send_evaluator_fixture_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - encapsulates agent creation and registration
2. Check for clarity: Good - clear two-step process (create, register)
3. Check for edge cases: NULL checks present, cleanup on registration failure
4. Check for memory leaks: Agent now properly owned by agency - leak from 0.7.1 is fixed!

**Actions**: No refactoring needed - implementation is clean.

**Verification**: All seven tests still pass, zero memory leaks (leak from 0.7.1 now resolved).

**Run**: `make ar_send_evaluator_fixture_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_evaluator_fixture_tests.log`

---

### Iteration 0.8: Refactor existing tests to use fixture helper (REFACTOR only)

**Review Status**: ✅ COMMITTED

**Objective**: Update all 7 existing send evaluator tests to use the new fixture helper.

**Status**: ✅ COMPLETED - All 7 tests refactored to use ar_send_evaluator_fixture helpers. All tests pass with zero memory leaks.

#### Implementation

**Step 1**: Add include to test file
```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Add after existing includes:
#include "ar_send_evaluator_fixture.h"
```

**Step 2**: Update each test function (example for first test)
```c
// File: modules/ar_send_instruction_evaluator_tests.c
// BEFORE:
static void test_send_instruction_evaluator__create_destroy(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_send_create_destroy");
    assert(own_fixture != NULL);

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(own_fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(own_fixture);

    ar_send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        ref_log, ref_expr_eval, mut_agency, ref_delegation
    );

    assert(own_evaluator != NULL);

    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

// AFTER:
static void test_send_instruction_evaluator__create_destroy(void) {
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_send_create_destroy");
    assert(own_fixture != NULL);

    // Use fixture helper - much simpler!
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_evaluator_fixture__create_evaluator(own_fixture);
    assert(own_evaluator != NULL);

    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}
```

**Step 3**: Repeat for all 7 existing tests:
1. `test_send_instruction_evaluator__create_destroy`
2. `test_send_instruction_evaluator__evaluate_with_instance`
3. `test_instruction_evaluator__evaluate_send_integer_message`
4. `test_instruction_evaluator__evaluate_send_string_message`
5. `test_instruction_evaluator__evaluate_send_with_result`
6. `test_instruction_evaluator__evaluate_send_memory_reference`
7. `test_instruction_evaluator__evaluate_send_invalid_args`

**Verification**:
```bash
# Ensure all existing tests still pass after refactoring
make ar_send_instruction_evaluator_tests 2>&1
grep "All send instruction_evaluator tests passed!" bin/run-tests/ar_send_instruction_evaluator_tests.log

# Verify no memory leaks
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_instruction_evaluator_tests.log
# Expected: 0 (0 bytes)
```

**Note**: This is pure refactoring - no behavior changes, just code cleanup.

---

### Iteration 1.1: Send to delegate returns true (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Test that `send(-1, message)` returns true (routing succeeds).

#### RED Phase

**Step 1**: Add AR_ASSERT header (if not already present)
```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Add to includes section (after existing includes):
#include "ar_assert.h"
```

**Step 2**: Add test function
```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Add new test function

static void test_send_instruction_evaluator__routes_to_delegate(void) {
    // Given a send evaluator fixture with a registered delegate
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_route_to_delegate");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Register delegate using fixture helper
    ar_delegate_t *delegate = ar_send_evaluator_fixture__create_and_register_delegate(fixture, -1, "test_delegate");
    AR_ASSERT(delegate != NULL, "Delegate registration should succeed");

    // Create evaluator and frame using fixture
    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    // When creating a send AST node for "send(-1, \"test message\")"
    const char *args[] = {"-1", "\"test message\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    AR_ASSERT(ast != NULL, "AST creation should succeed");

    // Create and attach expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument list creation should succeed");

    ar_expression_ast_t *delegate_id_ast = ar_expression_ast__create_literal_int(-1);
    AR_ASSERT(delegate_id_ast != NULL, "Delegate ID AST creation should succeed");
    ar_list__add_last(arg_asts, delegate_id_ast);

    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("test message");
    AR_ASSERT(msg_ast != NULL, "Message AST creation should succeed");
    ar_list__add_last(arg_asts, msg_ast);

    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    AR_ASSERT(ast_set == true, "Setting function arguments should succeed");

    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then it should succeed (negative ID should route to delegation)
    AR_ASSERT(result == true, "Send to delegate should succeed");  // ← FAILS (routes to agency instead, returns false for non-existent agent -1)

    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Expected RED**: Test compiles but FAILS at marked assertion because negative IDs currently route to agency (which returns false for non-existent agent -1).

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### GREEN Phase

**Minimal Implementation** - Just enough to pass the assertion (hardcoded return):

```zig
// File: modules/ar_send_instruction_evaluator.zig
// Modify evaluate function (lines 124-134)

// Send the message based on ID sign
var send_result: bool = undefined;
if (agent_id == 0) {
    // Special case: agent_id 0 is a no-op that always returns true
    std.debug.print("DEBUG [SEND_EVAL]: Sending to agent 0 - destroying message type={}\n", .{c.ar_data__get_type(own_message)});
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);
    send_result = true;
} else if (agent_id > 0) {
    // Positive IDs route to agency (agents)
    send_result = c.ar_agency__send_to_agent(ref_evaluator.?.ref_agency, agent_id, own_message);
} else {
    // Negative IDs: hardcoded return true for now
    // Actual delegation routing will be tested in Iteration 1.2
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);
    send_result = true;  // ← HARDCODED!
}
```

**Why this is minimal**:
- Only passes THIS iteration's assertion (`result == true`)
- Doesn't implement actual delivery yet
- Next iteration (1.2) will test actual delivery and force real implementation
- Message is destroyed to prevent leaks

**Expected GREEN**: Test PASSES - send returns true for negative IDs.

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - straightforward return path
2. Check for clarity: Good - clearly marked as temporary
3. Check for edge cases: Message cleanup prevents leaks

**Actions**: No refactoring needed - implementation is intentionally minimal.

**Verification**: Test passes, zero memory leaks.

**Run**: `make ar_send_instruction_evaluator_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_instruction_evaluator_tests.log`

---

### Iteration 1.2: Delegate receives message (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Verify that delegate actually receives and queues the message (forces real routing implementation).

#### RED Phase

```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Modify existing test function by adding new assertion

static void test_send_instruction_evaluator__routes_to_delegate(void) {
    // ... (all previous setup code remains the same) ...

    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    AR_ASSERT(result == true, "Send to delegate should succeed");

    // Then the delegate should have received the message
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    bool has_messages = ar_delegation__delegate_has_messages(delegation, -1);
    AR_ASSERT(has_messages == true, "Delegate should have received message");  // ← FAILS (new assertion)

    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Expected RED**: Test compiles but FAILS at new assertion because Iteration 1.1's hardcoded `true` return doesn't actually route to delegation - message is just destroyed!

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### GREEN Phase

**Now implement real delegation routing** - This assertion forces proper implementation:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// Modify evaluate function (lines 124-134) - REPLACE Iteration 1.1's hardcoded return

// Send the message based on ID sign
var send_result: bool = undefined;
if (agent_id == 0) {
    // Special case: agent_id 0 is a no-op that always returns true
    std.debug.print("DEBUG [SEND_EVAL]: Sending to agent 0 - destroying message type={}\n", .{c.ar_data__get_type(own_message)});
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);
    send_result = true;
} else if (agent_id > 0) {
    // Positive IDs route to agency (agents)
    send_result = c.ar_agency__send_to_agent(ref_evaluator.?.ref_agency, agent_id, own_message);
} else {
    // Negative IDs route to delegation (delegates)
    send_result = c.ar_delegation__send_to_delegate(ref_evaluator.?.ref_delegation, agent_id, own_message);
}
```

**Why now**: Iteration 1.2's new assertion (`has_messages == true`) forces us to actually route the message to delegation instead of just destroying it.

**Expected GREEN**: Test PASSES - message now routes to delegation and is queued.

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**:
1. Check for code duplication: None - routing logic is clean
2. Check for clarity: Good - three distinct cases with clear intent
3. Check for edge cases: Will be tested in Iterations 1.3-1.5

**Actions**: No refactoring needed - implementation is clean.

**Verification**: Both Iterations 1.1-1.2 tests pass, zero memory leaks.

**Run**: `make ar_send_instruction_evaluator_tests 2>&1 && grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_instruction_evaluator_tests.log`

---

### Iteration 1.3.1: Message is actually queued (not just accepted) (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Verify that the message is actually stored in the delegate's queue (not just discarded). This iteration proves the assertion catches real failures by temporarily breaking the implementation.

#### RED Phase

Add new assertion that will FAIL because we temporarily prevent queuing:

```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Modify existing test function by adding new assertion

static void test_send_instruction_evaluator__routes_to_delegate(void) {
    // ... (all previous setup and send code remains) ...

    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    AR_ASSERT(result == true, "Send to delegate should succeed");

    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    bool has_messages = ar_delegation__delegate_has_messages(delegation, -1);
    AR_ASSERT(has_messages == true, "Delegate should have received message");

    // NEW ASSERTION: Verify message actually exists and can be retrieved (not NULL)
    ar_data_t *own_received = ar_delegation__take_delegate_message(delegation, -1);
    AR_ASSERT(own_received != NULL, "Should be able to retrieve the queued message from delegate");  // ← WILL FAIL

    // Cleanup
    ar_data__destroy(own_received);
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Temporary Implementation to Verify RED Failure**:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// TEMPORARILY break routing to verify assertion fails

} else {
    // TEMPORARY: Don't actually queue - just destroy message
    // This makes the test fail so we can verify the assertion catches the failure
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);
    send_result = true;
    // ^ Assertion MUST fail: take_delegate_message() will return NULL
}
```

**Expected RED**: Test compiles but FAILS at new assertion because message was destroyed instead of queued (take_delegate_message returns NULL).

**Verify**: `make ar_send_instruction_evaluator_tests 2>&1` → assertion fails with "Should be able to retrieve..."

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### GREEN Phase

Remove the temporary destruction and restore real delegation routing:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// NOW: Real routing - queue the message properly

} else {
    // Negative IDs route to delegation (delegates)
    send_result = c.ar_delegation__send_to_delegate(ref_evaluator.?.ref_delegation, agent_id, own_message);
}
```

**Expected GREEN**: Test PASSES - message is now queued, retrieve succeeds.

**Verification**: Test should PASS - assertion succeeds because take_delegate_message() returns the queued message.

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**: No refactoring needed - assertion validates message persistence and we've proven it catches real failures.

---

### Iteration 1.3.2: Message type is preserved through routing (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Verify that the message type isn't lost or corrupted during the send/route process. This iteration proves the assertion catches type corruption bugs.

#### RED Phase

Add new assertion that will FAIL because we temporarily corrupt the type in the implementation:

```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Modify existing test function by adding new assertion

static void test_send_instruction_evaluator__routes_to_delegate(void) {
    // ... (all previous code remains) ...

    ar_data_t *own_received = ar_delegation__take_delegate_message(delegation, -1);
    AR_ASSERT(own_received != NULL, "Should be able to retrieve the queued message from delegate");

    // NEW ASSERTION: Verify message type is correct (not corrupted to wrong type)
    ar_data_type_t received_type = ar_data__get_type(own_received);
    AR_ASSERT(received_type == AR_DATA_TYPE__STRING, "Message type should be STRING, but got something else");  // ← WILL FAIL

    // Cleanup
    ar_data__destroy(own_received);
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Temporary Implementation to Verify RED Failure**:

Temporarily modify the implementation code to corrupt the message type BEFORE queuing. Since `ar_data_t` fields are private in test code, we corrupt in the implementation where we have access:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// TEMPORARY corruption in the routing code

} else {
    // Negative IDs route to delegation (delegates)
    // TEMPORARY: Create a corrupted message with wrong type to verify assertion catches it
    const corrupted_message = c.ar_data__create_int(42);  // Wrong type!
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);  // Destroy original
    send_result = c.ar_delegation__send_to_delegate(ref_evaluator.?.ref_delegation, agent_id, corrupted_message);
    // Expected RED: Assertion FAILS - received INT instead of STRING
}
```

**Expected RED**: Test compiles but FAILS at type assertion because message was replaced with INT type instead of preserving original STRING type.

**Verify**: `make ar_send_instruction_evaluator_tests 2>&1` → assertion fails with "Message type should be STRING, but got something else"

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### GREEN Phase

Remove the temporary type corruption from the implementation:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// REMOVE corruption - restore proper routing

} else {
    // Negative IDs route to delegation (delegates)
    // NOW: Route original message with correct type preserved
    send_result = c.ar_delegation__send_to_delegate(ref_evaluator.?.ref_delegation, agent_id, own_message);
}
```

**Expected GREEN**: Test PASSES - message type is preserved as STRING through the routing chain.

**Verification**: Assertion succeeds because type integrity is maintained (no corruption).

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**: No refactoring needed - assertion validates type integrity and we've proven it catches type corruption.

---

### Iteration 1.3.3: Message content is preserved through routing (RED-GREEN-REFACTOR)

**Review Status**: ✅ COMMITTED

**Objective**: Verify that the message content/value isn't lost or corrupted during send/route. This iteration proves the assertion catches content corruption bugs.

#### RED Phase

Add new assertion that will FAIL because we temporarily corrupt the content in the implementation:

```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Modify existing test function by adding new assertion

static void test_send_instruction_evaluator__routes_to_delegate(void) {
    // ... (all previous code remains) ...

    AR_ASSERT(received_type == AR_DATA_TYPE__STRING, "Message type should be STRING, but got something else");

    // NEW ASSERTION: Verify message content matches what we sent
    const char *received_string = ar_data__get_string(own_received);
    AR_ASSERT(strcmp(received_string, "test message") == 0, "Message content should be 'test message' but got different content");  // ← WILL FAIL

    // Cleanup
    ar_data__destroy(own_received);
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Temporary Implementation to Verify RED Failure**:

Temporarily modify the implementation code to corrupt the message content BEFORE queuing:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// TEMPORARY corruption in the routing code

} else {
    // Negative IDs route to delegation (delegates)
    // TEMPORARY: Replace message with wrong content to verify assertion catches it
    const corrupted_message = c.ar_data__create_string("wrong message");  // Wrong content!
    c.ar_data__destroy_if_owned(own_message, ref_evaluator);  // Destroy original
    send_result = c.ar_delegation__send_to_delegate(ref_evaluator.?.ref_delegation, agent_id, corrupted_message);
    // Expected RED: Assertion FAILS - received "wrong message" instead of "test message"
}
```

**Expected RED**: Test compiles but FAILS at content assertion because message was replaced with "wrong message" instead of preserving original "test message".

**Verify**: `make ar_send_instruction_evaluator_tests 2>&1` → assertion fails with "Message content should be 'test message' but got different content"

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### GREEN Phase

Remove the temporary content corruption from the implementation:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// REMOVE corruption - restore proper routing

} else {
    // Negative IDs route to delegation (delegates)
    // NOW: Route original message with content preserved
    send_result = c.ar_delegation__send_to_delegate(ref_evaluator.?.ref_delegation, agent_id, own_message);
}
```

**Expected GREEN**: Test PASSES - message content is preserved as "test message" through the routing chain.

**Verification**: Assertion succeeds because content integrity is maintained (no corruption).

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**: No refactoring needed - assertion validates content integrity and we've proven it catches content corruption bugs (memory corruption, truncation, modification).

---

### Iteration 2: Verify agent routing still works for positive IDs (RED-GREEN-REFACTOR)

**Review Status**: IMPLEMENTED

**Objective**: Ensure refactored routing doesn't break existing agent routing.

**Iteration Type**: VERIFICATION (tests existing functionality from Iteration 1)

#### RED Phase

**RED Phase has TWO goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED - even for verification iterations)
- **Purpose**: Prove this test can actually catch routing bugs
- **Method**: Apply temporary corruption, verify test FAILS
- **Status**: Implementation already exists from Iteration 1, but test validity must still be proven

**GOAL 2: Identify What to Implement** (CONDITIONAL - already satisfied for verification iterations)
- **Purpose**: Determine what code needs to be written
- **Method**: Observe what failing test expects
- **Status**: ✅ ALREADY SATISFIED - Agent routing implemented in Iteration 1

**CRITICAL**: Both goals are independent. Even though Goal 2 is satisfied (implementation exists), Goal 1 is STILL REQUIRED to prove the test works correctly.

---

**Step 1: Write the Test** (same for both new and verification iterations)

```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Add new test function

static void test_send_instruction_evaluator__routes_to_agent(void) {
    // Given a send evaluator fixture with a registered agent
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_route_to_agent");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Register agent using fixture helper
    ar_agent_t *agent = ar_send_evaluator_fixture__create_and_register_agent(fixture, 1, NULL);
    AR_ASSERT(agent != NULL, "Agent registration should succeed");

    // Create evaluator and frame using fixture
    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    // When creating a send AST node for "send(1, \"agent message\")"
    const char *args[] = {"1", "\"agent message\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    AR_ASSERT(ast != NULL, "AST creation should succeed");

    // Create and attach expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument list creation should succeed");

    ar_expression_ast_t *agent_id_ast = ar_expression_ast__create_literal_int(1);
    AR_ASSERT(agent_id_ast != NULL, "Agent ID AST creation should succeed");
    ar_list__add_last(arg_asts, agent_id_ast);

    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("agent message");
    AR_ASSERT(msg_ast != NULL, "Message AST creation should succeed");
    ar_list__add_last(arg_asts, msg_ast);

    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    AR_ASSERT(ast_set == true, "Setting function arguments should succeed");

    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then it should succeed (positive ID should route to agency)
    AR_ASSERT(result == true, "Send to agent should succeed");  // ← Test ensures routing logic works

    // And the agent should have received the message
    ar_agency_t *agency = ar_send_evaluator_fixture__get_agency(fixture);
    bool has_messages = ar_agency__agent_has_messages(agency, 1);
    AR_ASSERT(has_messages == true, "Agent should have received message");

    // Verify message content and type
    ar_data_t *own_received = ar_agency__take_agent_message(agency, 1);
    AR_ASSERT(own_received != NULL, "Should be able to retrieve message from agent");
    AR_ASSERT(ar_data__get_type(own_received) == AR_DATA_TYPE__STRING, "Message should be STRING type");
    AR_ASSERT(strcmp(ar_data__get_string(own_received), "agent message") == 0,
              "Message content should match sent value");

    // Cleanup
    ar_data__destroy(own_received);
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**This step is REQUIRED even though implementation exists. It proves the test can catch bugs.**

Temporarily break agent routing to verify the assertion catches routing failures:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// TEMPORARY: Route positive IDs to wrong destination

} else if (agent_id > 0) {
    // TEMPORARY: Route to delegation instead of agency (WRONG!)
    // This makes the test fail so we can verify the assertion catches it
    send_result = c.ar_delegation__send_to_delegate(
        ref_evaluator.?.ref_delegation,
        agent_id,  // Wrong - positive ID should go to agency, not delegation
        own_message
    );
    // Expected RED: Test FAILS - agent doesn't receive message (routed to wrong place)
}
```

**Expected RED**: Test compiles but FAILS because positive IDs are routed to delegation instead of agency - agent won't receive the message.

**Verify**: `make ar_send_instruction_evaluator_tests 2>&1` → assertion fails with "Agent should have received message"

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

**Evidence of Goal 1 completion**: Test output showing FAILURE with message "Agent should have received message"

---

#### GREEN Phase

**GREEN Phase Goal: Make Test Pass**

For **verification iterations** like this one:
- **Goal 2** is already satisfied (implementation exists from Iteration 1)
- Simply remove temporary corruption from Goal 1
- Test should immediately PASS

For **new implementation iterations**:
- **Goal 2** needs satisfaction (write minimal implementation)
- Use test failure to guide what to implement

**This iteration**: Remove temporary corruption and verify existing routing works:

```zig
// File: modules/ar_send_instruction_evaluator.zig
// REMOVE corruption - restore proper agent routing

} else if (agent_id > 0) {
    // Positive IDs route to agency (agents) - CORRECT routing restored
    send_result = c.ar_agency__send_to_agent(ref_evaluator.?.ref_agency, agent_id, own_message);
}
```

**Verification**: Test PASSES - agent routing works correctly.

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**: No refactoring needed - this iteration validates existing behavior.

---

### Iteration 3: Handle non-existent delegate gracefully (RED-GREEN-REFACTOR)

**Review Status**: IMPLEMENTED

**Objective**: Verify that sending to a non-registered delegate ID returns false.

#### RED Phase
```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Add new test function

static void test_send_instruction_evaluator__nonexistent_delegate_returns_false(void) {
    // Given a send evaluator fixture WITHOUT registering any delegates
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_nonexistent_delegate");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Create evaluator and frame using fixture
    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    // When creating a send AST node for "send(-99, \"message\")" (delegate -99 doesn't exist)
    const char *args[] = {"-99", "\"message\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    AR_ASSERT(ast != NULL, "AST creation should succeed");

    // Create and attach expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument list creation should succeed");

    ar_expression_ast_t *delegate_id_ast = ar_expression_ast__create_literal_int(-99);
    AR_ASSERT(delegate_id_ast != NULL, "Delegate ID AST creation should succeed");
    ar_list__add_last(arg_asts, delegate_id_ast);

    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("message");
    AR_ASSERT(msg_ast != NULL, "Message AST creation should succeed");
    ar_list__add_last(arg_asts, msg_ast);

    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    AR_ASSERT(ast_set == true, "Setting function arguments should succeed");

    // When evaluating the send to non-existent delegate
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then it should fail (return false) for non-existent delegate
    AR_ASSERT(result == false, "Send to non-existent delegate should return false");  // ← Test verifies error handling

    // Verify no messages were queued anywhere
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    bool delegate_has_messages = ar_delegation__delegate_has_messages(delegation, -99);
    AR_ASSERT(delegate_has_messages == false, "Non-existent delegate should have no messages");

    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}
```

**Temporary Implementation to Verify RED Failure**:

Temporarily modify delegation code to accept non-existent delegates to verify the assertion catches error handling failures:

```c
// File: modules/ar_delegation.c (or wherever ar_delegation__send_to_delegate is implemented)
// TEMPORARY: Return true for non-existent delegate to make test fail

bool ar_delegation__send_to_delegate(...) {
    // TEMPORARY: Don't check if delegate exists - always succeed
    // This makes the test fail so we can verify the assertion catches error handling bugs
    ar_data__destroy(own_message);  // Still destroy message
    return true;  // Wrong - should return false for non-existent delegate
    // Expected RED: Test FAILS - expects false but gets true
}
```

**Expected RED**: Test compiles but FAILS because non-existent delegate returns true instead of false - error handling is broken.

**Verify**: `make ar_send_instruction_evaluator_tests 2>&1` → assertion fails with "Send to non-existent delegate should return false"

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### GREEN Phase

Remove temporary bypass and restore proper error handling:

```c
// File: modules/ar_delegation.c
// REMOVE bypass - restore proper error checking

bool ar_delegation__send_to_delegate(...) {
    // Check if delegate exists
    ar_delegate_t *delegate = ar_delegation__get_delegate(delegation, delegate_id);
    if (!delegate) {
        // Delegate not found - destroy message and return false
        ar_data__destroy(own_message);
        return false;  // Correct error handling restored
    }
    // ... rest of implementation queues message
}
```

**Verification**: Test PASSES - error handling works correctly (returns false for non-existent delegate).

**Run**: `make ar_send_instruction_evaluator_tests 2>&1`

#### REFACTOR Phase

**Analysis**: No refactoring needed - this iteration validates error handling.

---

### Iteration 4: Update test main() and documentation (RED-GREEN-REFACTOR)

**Review Status**: IMPLEMENTED

**Objective**: Add new tests to main() and update module documentation.

#### RED Phase

N/A - This is a documentation/integration iteration, not a behavior test.

#### GREEN Phase

**Action 1**: Add new test calls to main()
```c
// File: modules/ar_send_instruction_evaluator_tests.c
// Update main() function (around line 348)

int main(void) {
    printf("Starting send instruction_evaluator tests...\n");

    // Existing tests...
    test_send_instruction_evaluator__create_destroy();
    printf("test_send_instruction_evaluator__create_destroy passed!\n");

    test_send_instruction_evaluator__evaluate_with_instance();
    printf("test_send_instruction_evaluator__evaluate_with_instance passed!\n");

    test_instruction_evaluator__evaluate_send_integer_message();
    printf("test_instruction_evaluator__evaluate_send_integer_message passed!\n");

    test_instruction_evaluator__evaluate_send_string_message();
    printf("test_instruction_evaluator__evaluate_send_string_message passed!\n");

    test_instruction_evaluator__evaluate_send_with_result();
    printf("test_instruction_evaluator__evaluate_send_with_result passed!\n");

    test_instruction_evaluator__evaluate_send_memory_reference();
    printf("test_instruction_evaluator__evaluate_send_memory_reference passed!\n");

    test_instruction_evaluator__evaluate_send_invalid_args();
    printf("test_instruction_evaluator__evaluate_send_invalid_args passed!\n");

    // NEW TESTS FOR TDD CYCLE 7
    test_send_instruction_evaluator__routes_to_delegate();
    printf("test_send_instruction_evaluator__routes_to_delegate passed!\n");

    test_send_instruction_evaluator__routes_to_agent();
    printf("test_send_instruction_evaluator__routes_to_agent passed!\n");

    test_send_instruction_evaluator__nonexistent_delegate_returns_false();
    printf("test_send_instruction_evaluator__nonexistent_delegate_returns_false passed!\n");

    printf("All send instruction_evaluator tests passed!\n");

    return 0;
}
```

**Action 2**: Update module documentation
```markdown
<!-- File: modules/ar_send_instruction_evaluator.md -->
<!-- Add new section after existing routing description -->

## Message Routing Architecture

The send instruction evaluator implements **ID-based message routing** following the pattern documented in [id-based-message-routing-pattern.md](../kb/id-based-message-routing-pattern.md):

### Routing Rules

| Target ID | Destination | Function Called |
|-----------|-------------|-----------------|
| `0` | No-op (message destroyed) | N/A - returns `true` |
| `> 0` | Agent via agency | `ar_agency__send_to_agent()` |
| `< 0` | Delegate via delegation | `ar_delegation__send_to_delegate()` |

### Example Usage

```c
// Send to agent ID 1
send(1, "message to agent")

// Send to delegate ID -1 (e.g., FileDelegate)
send(-1, {"action": "read", "path": "data.txt"})

// Send to agent 0 (no-op, always succeeds)
send(0, "discarded message")
```

### Error Handling

- **Non-existent agent/delegate**: Returns `false`, message is destroyed
- **Invalid agent_id expression**: Returns `false`, message is destroyed
- **Message ownership**: Evaluator claims or copies message before routing
- **Result assignment**: If present, stores `1` (true) or `0` (false) in memory
```

**Verification**: Run `make check-docs` to validate documentation updates.

#### REFACTOR Phase

**Analysis**: Check for any remaining debug output that should be removed or updated.

**Action**: Review and clean up any temporary debug statements.

---

## Success Criteria

### ✅ Cycle 0: Fixture Infrastructure - COMPLETED

**Fixture Tests (9 tests)**:
- [x] All 9 fixture tests pass (0.1-0.8 coverage)
- [x] All tests follow BDD structure with Given/When/Then
- [x] Zero memory leaks confirmed

**Existing Tests Refactored (7 tests)**:
- [x] All 7 existing `ar_send_instruction_evaluator` tests refactored to use fixture helpers
- [x] All refactored tests pass without behavior changes
- [x] Zero memory leaks confirmed

**Total**: 16 tests passing (9 fixture + 7 refactored)

### ⏭️ Next Phase: Cycle 1 - Message Delegation Routing

- [ ] Three new tests pass:
  - `test_send_instruction_evaluator__routes_to_delegate` - Negative ID routing
  - `test_send_instruction_evaluator__routes_to_agent` - Positive ID routing
  - `test_send_instruction_evaluator__nonexistent_delegate_returns_false` - Error handling
- [ ] Zero memory leaks: `grep "Actual memory leaks: 0" bin/run-tests/memory_report_ar_send_instruction_evaluator_tests.log`
- [ ] Build passes: `make clean build 2>&1 | tee build.log && grep -i "error" build.log` (should be empty)
- [ ] Documentation updated and validated: `make check-docs`

## Verification Steps

After completing all iterations:

```bash
# 1. Build and run tests
make ar_send_instruction_evaluator_tests 2>&1

# 2. Check memory leaks
grep "Actual memory leaks:" bin/run-tests/memory_report_ar_send_instruction_evaluator_tests.log

# 3. Expected output: "0 (0 bytes)"

# 4. Verify all 10 tests pass (7 existing + 3 new)
grep -c "passed!" bin/run-tests/ar_send_instruction_evaluator_tests.log
# Expected: 10

# 5. Validate documentation
make check-docs

# 6. Full build verification
make clean build 2>&1 | tee build.log

# 7. Run full test suite
make run-tests 2>&1 | tee test_suite.log

# 8. Check for any regression
grep "tests passed" test_suite.log
# Expected: All tests passing (should be 81 tests: 78 existing + 3 new)
```

## Rollback Strategy

If issues arise during implementation:

```bash
# 1. Stash changes
git stash save "TDD Cycle 7 - partial work"

# 2. Verify clean state
make clean build 2>&1
make run-tests 2>&1

# 3. Review stashed changes
git stash show -p

# 4. Restore if needed
git stash pop
```

## Files Modified

### Test Infrastructure (Iteration 0)
1. `modules/ar_send_evaluator_fixture.h` (NEW) - Fixture helper header
2. `modules/ar_send_evaluator_fixture.c` (NEW) - Fixture helper implementation
3. `modules/ar_send_instruction_evaluator_tests.c` - Update existing tests to use fixture helper

### Production Code (Iteration 1.1)
4. `modules/ar_send_instruction_evaluator.zig` (lines 124-134) - Add ID-based routing logic

### Test Code (Iterations 1.1-3)
5. `modules/ar_send_instruction_evaluator_tests.c` - Add 3 new test functions + update main()

### Documentation (Iteration 4)
6. `modules/ar_send_instruction_evaluator.md` - Add routing architecture section

## Dependencies

### KB Articles
- [ID-Based Message Routing Pattern](../kb/id-based-message-routing-pattern.md)
- [Message Ownership Flow](../kb/message-ownership-flow.md)
- [TDD Iteration Planning Pattern](../kb/tdd-iteration-planning-pattern.md)
- [TDD RED Phase Assertion Requirement](../kb/tdd-red-phase-assertion-requirement.md)
- [AR_ASSERT for Descriptive Failures](../kb/ar-assert-descriptive-failures.md)
- [Test Assertion Strength Patterns](../kb/test-assertion-strength-patterns.md)

### Modules
- `ar_delegation` (peer to ar_agency, manages delegates)
- `ar_agency` (manages agents, existing behavior)
- `ar_delegate` (message queue infrastructure from TDD Cycle 6.5)

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Breaking existing agent routing | HIGH | Iteration 2 explicitly tests positive IDs; all existing tests use agent_id=0 or positive |
| Memory leaks in error paths | MEDIUM | ar_delegation__send_to_delegate() handles message cleanup on failure |
| Fixture doesn't support delegates | MEDIUM | ar_evaluator_fixture already provides get_delegation() (verified in existing test code) |

## Next Steps After Completion

After TDD Cycle 7 completes successfully:

1. **Commit** with message referencing TDD Cycle 7 completion
2. **Update TODO.md**: Mark TDD Cycle 7 complete, note TDD Cycle 8 is next
3. **Move to TDD Cycle 8**: Begin FileDelegate implementation (first built-in delegate)
4. **Update CHANGELOG.md**: Add entry for delegate routing completion

## Estimated Timeline

### ✅ Cycle 0 - COMPLETED (2-3 hours actual)
- Iteration 0.1-0.8 (Fixture infrastructure): Completed with strict RED-GREEN-REFACTOR discipline
- Iteration 0.8 (Refactor existing tests): Completed - 7 tests refactored to use fixture helpers
- **Cycle 0 Total**: All 9 iterations implemented with zero memory leaks

### ⏭️ Cycle 1 - Remaining (2-3 hours estimated)
- Iteration 1.1 (Send returns true): 15-20 minutes
- Iteration 1.2 (Has messages): 10-15 minutes
- Iteration 1.3.1 (Message queued): 10-15 minutes
- Iteration 1.3.2 (Message type preserved): 10-15 minutes
- Iteration 1.3.3 (Message content preserved): 10-15 minutes
- Iteration 2 (Agent routing verification): 30-45 minutes
- Iteration 3 (Error handling): 20-30 minutes
- Iteration 4 (Documentation): 30-45 minutes
- **Cycle 1 Total**: ~2-3 hours

- **Overall Project**: ~5-6 hours (on track)

## Notes

- **No commits during iterations**: Commit only after ALL 4 iterations complete
- **Minimal GREEN implementations**: Don't add features not tested
- **Refactor is MANDATORY**: Even if "No improvements identified", must explicitly state this
- **Debug output**: Keep or remove debug print statement at line 128 based on preference (currently helps with tracing)
