# TDD Plan Iteration Split Pattern

## Learning
When reviewing TDD plans, iterations that test multiple behaviors should be split into sub-iterations (N.1, N.2) following TDD GREEN minimalism. The first sub-iteration (.1) tests for non-NULL return with minimal implementation (create only). The second sub-iteration (.2) verifies actual registration/integration behavior. Use decimal numbering rather than renumbering all subsequent iterations.

## Importance
Testing multiple assertions in one iteration violates TDD GREEN phase minimalism - you cannot implement "just enough to pass" when multiple behaviors are bundled together. This leads to over-implementation in early iterations and obscures the incremental nature of TDD. Splitting into .1/.2 sub-iterations maintains the one-assertion-per-iteration principle while avoiding error-prone cascading renumbering.

## Example
```c
// WRONG: Multiple assertions in one iteration
// Iteration 0.6: create_and_register_agent() works
static void test_fixture__create_and_register_agent(void) {
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    int64_t agent_id = ar_agent_store_fixture__create_agent(
        fixture, "echo", "1.0"
    );

    // Two assertions - tests too much!
    AR_ASSERT(agent_id > 0, "Should return agent ID");
    AR_ASSERT(ar_agent_store_fixture__verify_agent(fixture, agent_id, "echo"),
              "Should be registered");

    ar_agent_store_fixture__destroy(fixture);
}

// GREEN Phase: Must implement both creation AND registration at once
// This violates minimalism - we're implementing untested behavior


// CORRECT: Split into two sub-iterations using decimal numbering

// Iteration 0.6.1: Returns valid agent ID (minimal)
static void test_fixture__create_and_register_agent(void) {
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    int64_t agent_id = ar_agent_store_fixture__create_agent(
        fixture, "echo", "1.0"
    );

    // One assertion - tests minimal behavior
    AR_ASSERT(agent_id > 0, "Should return agent ID");  // ← FAILS (stub returns 0)

    // Temporary cleanup since not registered yet
    ar_agent_store_fixture__destroy_agent(fixture, agent_id);
    ar_agent_store_fixture__destroy(fixture);
}

// GREEN Phase 0.6.1: Allocate ID only (minimal implementation)
int64_t ar_agent_store_fixture__create_agent(
    ar_agent_store_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_method_version
) {
    if (!mut_fixture) return 0;

    // Minimal: just allocate and return ID (no registration yet)
    return ar_agent_store_fixture__get_next_agent_id(mut_fixture);
}


// Iteration 0.6.2: Agent is actually registered
static void test_fixture__create_and_register_agent(void) {
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    int64_t agent_id = ar_agent_store_fixture__create_agent(
        fixture, "echo", "1.0"
    );
    AR_ASSERT(agent_id > 0, "Should return agent ID");

    // New assertion - tests registration behavior
    AR_ASSERT(ar_agent_store_fixture__verify_agent(fixture, agent_id, "echo"),
              "Should be registered");  // ← FAILS (not registered in 0.6.1)

    // No manual cleanup - fixture owns agent now
    ar_agent_store_fixture__destroy(fixture);
}

// GREEN Phase 0.6.2: Add registration logic
int64_t ar_agent_store_fixture__create_agent(
    ar_agent_store_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_method_version
) {
    if (!mut_fixture) return 0;

    int64_t agent_id = ar_agent_store_fixture__get_next_agent_id(mut_fixture);

    // NOW: Create and register agent (ownership transfer)
    ar_agent_t *own_agent = ar_agent__create(ref_method_name, ref_method_version);
    if (!own_agent) return 0;

    bool registered = ar_agent_registry__register(
        get_registry(mut_fixture), agent_id, own_agent
    );
    if (!registered) {
        ar_agent__destroy(own_agent);
        return 0;
    }

    return agent_id;
}
```

## Generalization

### When to Split Iterations

Split an iteration into .1/.2 sub-iterations when:
1. The function name suggests two operations (e.g., "create_and_register")
2. The test has two distinct AR_ASSERT calls
3. GREEN phase would implement untested behavior to pass both assertions
4. One assertion checks object existence, another checks integration

### Splitting Protocol

**Step 1: Identify the split point**
- First assertion: Object creation/basic functionality
- Second assertion: Integration/registration/full behavior

**Step 2: Create .1 sub-iteration**
- Test: One assertion for basic functionality
- GREEN: Minimal implementation (create only, no integration)
- Cleanup: Add temporary manual cleanup (see [Temporary Test Cleanup Pattern](temporary-test-cleanup-pattern.md))

**Step 3: Create .2 sub-iteration**
- Test: Keep first assertion, add integration assertion
- GREEN: Add integration logic (registration, ownership transfer)
- Cleanup: Remove temporary cleanup from .1

**Step 4: Use decimal numbering**
- Don't renumber subsequent iterations
- Original 0.7 stays 0.7, original 0.8 stays 0.8
- Localized change makes diffs easier to review

### Common Split Patterns

```bash
# Pattern: create_and_X functions
create_and_register() → .1 (create) + .2 (register)
create_and_initialize() → .1 (create) + .2 (initialize)
create_and_connect() → .1 (create) + .2 (connect)

# Pattern: Existence + Integration
.1: Object != NULL
.2: Object is registered/connected/linked

# Pattern: Success + Verification
.1: Function returns success
.2: Side effects are verified
```

## Implementation

### Detection During Plan Review

```bash
# Red flags that suggest splitting needed:
1. Function name has "and" in it
2. Multiple AR_ASSERT in same test
3. Comments like "// Test creation and registration"
4. GREEN phase has multiple unrelated steps
```

### Splitting Checklist

```markdown
For iteration N that needs splitting:

- [ ] Identify the two behaviors being tested
- [ ] Create N.1 for first behavior (usually creation/basic function)
- [ ] Add temporary cleanup to N.1 test
- [ ] Create N.2 for second behavior (usually integration)
- [ ] Remove temporary cleanup from N.2 test
- [ ] Keep all subsequent iterations with original numbering
- [ ] Document why split was needed in plan
```

### Numbering Examples

```markdown
Original plan:
- 0.5: get_agency() works
- 0.6: create_and_register_delegate() works  ← Split this
- 0.7: create_and_register_agent() works     ← Split this
- 0.8: Refactor tests

After splitting (decimal numbering):
- 0.5: get_agency() works
- 0.6.1: create_and_register_delegate() returns non-NULL
- 0.6.2: Delegate is registered
- 0.7.1: create_and_register_agent() returns non-NULL
- 0.7.2: Agent is registered
- 0.8: Refactor tests  ← No renumbering needed!
```

## Related Patterns
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md)
- [TDD GREEN Phase Minimalism](tdd-green-phase-minimalism.md)
- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md)
- [Temporary Test Cleanup Pattern](temporary-test-cleanup-pattern.md)
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md)
