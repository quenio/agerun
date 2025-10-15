# Temporary Test Cleanup Pattern

## Learning
When TDD GREEN minimalism requires not implementing full functionality yet (e.g., creating but not registering objects), add temporary manual cleanup in tests to maintain zero memory leak policy. Mark cleanup as temporary with explicit comments, to be removed when the next iteration adds proper ownership transfer.

## Importance
Maintains zero memory leak policy even during minimal TDD implementations. Without temporary cleanup, .1 iterations (that create but don't register) would leak memory. The explicit "temporary" comment prevents confusion and ensures cleanup is removed when ownership transfer is implemented.

## Example
```c
// Iteration N.1: Function returns valid ID (minimal GREEN)

// RED Phase - Test with temporary cleanup
static void test_fixture__create_and_register_agent(void) {
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();
    AR_ASSERT(fixture != NULL, "Fixture should exist");

    // When creating agent
    int64_t agent_id = ar_agent_store_fixture__create_agent(
        fixture, "echo", "1.0"
    );

    // Then should return valid agent ID
    AR_ASSERT(agent_id > 0, "Should create agent");  // ← FAILS (stub returns 0)

    // Cleanup (temporary: manually destroy agent since not registered yet)
    ar_agent_store_fixture__destroy_agent(fixture, agent_id);
    ar_agent_store_fixture__destroy(fixture);
}

// GREEN Phase - Minimal implementation (allocate ID only, no registration)
int64_t ar_agent_store_fixture__create_agent(
    ar_agent_store_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_method_version
) {
    if (!mut_fixture) return 0;

    // Minimal: allocate and return ID (no registration = no ownership transfer)
    return ar_agent_store_fixture__get_next_agent_id(mut_fixture);
}


// Iteration N.2: Verify object is actually registered

// RED Phase - Temporary cleanup removed
static void test_fixture__create_and_register_agent(void) {
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();
    AR_ASSERT(fixture != NULL, "Fixture should exist");

    int64_t agent_id = ar_agent_store_fixture__create_agent(
        fixture, "echo", "1.0"
    );
    AR_ASSERT(agent_id > 0, "Should create agent");

    // Then agent should be registered
    AR_ASSERT(ar_agent_store_fixture__verify_agent(fixture, agent_id, "echo"),
              "Should be registered");  // ← FAILS (new assertion)

    // Cleanup (removed manual destroy_agent - fixture now owns it)
    ar_agent_store_fixture__destroy(fixture);
}

// GREEN Phase - Add registration (ownership transfer)
int64_t ar_agent_store_fixture__create_agent(
    ar_agent_store_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_method_version
) {
    if (!mut_fixture) return 0;

    int64_t agent_id = ar_agent_store_fixture__get_next_agent_id(mut_fixture);

    ar_agent_t *own_agent = ar_agent__create(ref_method_name, ref_method_version);
    if (!own_agent) return 0;

    // NOW: Register (ownership transfers to registry)
    bool registered = ar_agent_registry__register(
        get_registry(mut_fixture), agent_id, own_agent
    );
    if (!registered) {
        ar_agent__destroy(own_agent);
        return 0;
    }

    // Return ID - registry owns the agent
    return agent_id;
}
```

## Generalization

### When Temporary Cleanup is Needed

Add temporary test cleanup when:
1. **Minimal GREEN doesn't transfer ownership** (e.g., creates but doesn't register)
2. **Object is returned but not owned** by a parent structure yet
3. **Next iteration will add ownership transfer** (registration, insertion into collection)
4. **Without cleanup, memory would leak** in the .1 iteration

### Cleanup Pattern Template

```c
// Pattern for .1 iteration (create only)
static void test_function_name(void) {
    // Given/When setup
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();
    int64_t agent_id = function_under_test(fixture, params);

    // Then verify creation
    AR_ASSERT(agent_id > 0, "Should create agent");

    // Cleanup (temporary: manually destroy agent since not registered yet)
    ar_agent_store_fixture__destroy_agent(fixture, agent_id);  // ← Added for .1, removed in .2
    ar_agent_store_fixture__destroy(fixture);
}
```

### Comment Format (MANDATORY)

Always use this exact comment format to mark temporary cleanup:
```c
// Cleanup (temporary: manually destroy X since not registered yet)
```

This format:
- Makes it obvious the cleanup is temporary
- Explains why it's needed ("not registered yet")
- Indicates what to do in the next iteration (remove it)

### Removal Timing

Remove temporary cleanup when:
1. **Ownership transfer is implemented** (e.g., registration added)
2. **Parent structure now owns the object**
3. **Parent's destroy will handle cleanup**

Update the comment when removing:
```c
// Cleanup (removed manual ar_delegate__destroy - delegation now owns it)
parent__destroy(parent);  // This now destroys the delegate too
```

## Implementation

### Step-by-Step for .1 Iteration

```bash
# 1. Write test with temporary cleanup
cat > test.c << 'EOF'
int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");
AR_ASSERT(agent_id > 0, "Should create");

// Cleanup (temporary: manually destroy since not registered yet)
ar_agent_store_fixture__destroy_agent(fixture, agent_id);
ar_agent_store_fixture__destroy(fixture);
EOF

# 2. Implement minimal GREEN (allocate ID only)
cat > implementation.c << 'EOF'
int64_t ar_agent_store_fixture__create_agent(...) {
    // Just allocate ID - no registration yet
    return ar_agent_store_fixture__get_next_agent_id(mut_fixture);
}
EOF

# 3. Verify zero leaks
make test_name 2>&1
grep "Actual memory leaks:" bin/run-tests/memory_report_test_name.log
# Expected: 0 (0 bytes)
```

### Step-by-Step for .2 Iteration

```bash
# 1. Update test - remove temporary cleanup
cat > test.c << 'EOF'
int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");
AR_ASSERT(agent_id > 0, "Should create");
AR_ASSERT(ar_agent_store_fixture__verify_agent(fixture, agent_id, "echo"),
          "Should be registered");  // New assertion

// Cleanup (removed manual destroy_agent - fixture now owns it)
ar_agent_store_fixture__destroy(fixture);  // This destroys agent via registry
EOF

# 2. Add ownership transfer to GREEN
cat > implementation.c << 'EOF'
int64_t ar_agent_store_fixture__create_agent(...) {
    int64_t agent_id = ar_agent_store_fixture__get_next_agent_id(mut_fixture);

    ar_agent_t *own_agent = ar_agent__create(ref_method_name, ref_method_version);
    if (!own_agent) return 0;

    // NOW: Register (ownership transfer)
    ar_agent_registry__register(get_registry(mut_fixture), agent_id, own_agent);
    return agent_id;
}
EOF

# 3. Verify zero leaks (should still pass)
make test_name 2>&1
grep "Actual memory leaks:" bin/run-tests/memory_report_test_name.log
# Expected: 0 (0 bytes)
```

### Common Scenarios

```c
// Scenario 1: Data object created but not added to list yet
ar_data_t *own_item = ar_data__create_string("test");
AR_ASSERT(own_item != NULL, "Should create");
// Cleanup (temporary: manually destroy since not added to list yet)
ar_data__destroy(own_item);

// Scenario 2: Agent created but ID not returned yet
int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");
AR_ASSERT(agent_id > 0, "Should create");
// Cleanup (temporary: manually destroy since not registered yet)
ar_agent_store_fixture__destroy_agent(fixture, agent_id);

// Scenario 3: Delegate created but not registered yet
ar_delegate_t *own_delegate = ar_delegate__create(log, "test");
AR_ASSERT(own_delegate != NULL, "Should create");
// Cleanup (temporary: manually destroy since not registered yet)
ar_delegate__destroy(own_delegate);
```

## Verification

### Before Committing .1 Iteration

```bash
# Must pass: Zero memory leaks with temporary cleanup
make test_name && grep "Actual memory leaks: 0" bin/run-tests/memory_report_test_name.log
```

### Before Committing .2 Iteration

```bash
# Must pass: Zero memory leaks WITHOUT temporary cleanup
make test_name && grep "Actual memory leaks: 0" bin/run-tests/memory_report_test_name.log

# Verify temporary cleanup was removed
! grep "temporary.*destroy" modules/test_name.c
```

## Related Patterns
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md)
- [TDD GREEN Phase Minimalism](tdd-green-phase-minimalism.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
