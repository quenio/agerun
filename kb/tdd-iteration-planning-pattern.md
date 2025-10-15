# TDD Iteration Planning Pattern

**Note**: Examples use `ar_delegate__send()`, `ar_delegate__has_messages()`, and `ar_delegate__take_message()` which are functions planned for implementation in TDD Cycle 6.5, not yet implemented.  // EXAMPLE: TDD Cycle 6.5

## Learning
Each TDD iteration should test exactly ONE assertion. When planning TDD work, count the number of assertions needed, not the number of features. One assertion = one RED-GREEN-REFACTOR iteration.

## Importance
Testing multiple assertions per iteration violates the minimal GREEN implementation principle. You cannot implement "just enough" to pass if multiple behaviors are bundled together. This leads to over-implementation in early iterations and obscures the incremental nature of TDD.

## Example
```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// WRONG: Multiple assertions in one iteration plan
// Iteration 1: "Test that send() works"
// - Should return true
// - Should queue the message
// - Should transfer ownership
// GREEN: Must implement all three behaviors at once

// CORRECT: One assertion per iteration
// Iteration 1: send() returns true
static void test_delegate__send_returns_true(void) {
    // Given a delegate instance
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When sending a message
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegate__send(own_delegate, own_message);  // EXAMPLE: To be implemented in TDD Cycle 6.5

    // Then send should return true
    AR_ASSERT(result, "Send should return true");  // ONE assertion

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}

// Iteration 2: has_messages() returns false when empty
static void test_delegate__has_no_messages_initially(void) {
    // Given a delegate with no messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When checking if delegate has messages
    bool has_messages = ar_delegate__has_messages(own_delegate);  // EXAMPLE: To be implemented in TDD Cycle 6.5

    // Then it should return false
    AR_ASSERT(!has_messages, "Should have no messages initially");  // ONE assertion

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}

// Iteration 3: has_messages() returns true after send()
// ... and so on
```

## Generalization
When planning TDD cycles, apply this systematic approach:

1. **List all behaviors** needed for the feature
2. **Break each behavior** into individual assertions
3. **Create one iteration** per assertion
4. **Number the iterations** sequentially (Iteration 1, 2, 3...)
5. **Verify independence**: Each iteration should add exactly one new passing assertion

### Planning Checklist
For each iteration, verify:
- [ ] Exactly ONE new assertion is introduced
- [ ] RED phase can show that specific assertion failing
- [ ] GREEN phase can pass that assertion with minimal code
- [ ] Previous iterations' assertions still pass

### Common Mistake: Feature-Based Planning
```  // EXAMPLE: TDD Cycle 6.5 planned functions
WRONG:
- Iteration 1: Implement send()
- Iteration 2: Implement has_messages()
- Iteration 3: Implement take_message()

CORRECT:
- Iteration 1: send() returns true
- Iteration 2: has_messages() returns false when empty
- Iteration 3: has_messages() returns true after send()
- Iteration 4: take_message() returns NULL when empty
- Iteration 5: take_message() returns message after send()
- Iteration 6: take_message() removes message from queue
```

## Implementation
```bash  # EXAMPLE: TDD Cycle 6.5 planned functions
# Step 1: List all expected behaviors (from requirements/design)
# For ar_delegate message queue:
# - Send messages
# - Check if messages exist
# - Retrieve and remove messages

# Step 2: Convert to specific assertions
cat > assertions.txt << 'EOF'
1. ar_delegate__send() returns true
2. ar_delegate__has_messages() returns false initially
3. ar_delegate__has_messages() returns true after send
4. ar_delegate__take_message() returns NULL when empty
5. ar_delegate__take_message() returns message when available
6. ar_delegate__take_message() removes message from queue
EOF

# Step 3: Create one iteration per assertion
# Each line becomes one RED-GREEN-REFACTOR iteration

# Step 4: Estimate effort
wc -l assertions.txt  # 6 iterations = approximately 1-2 hours
```

### Effort Estimation
- Simple assertion (return value check): ~10-15 minutes per iteration
- Complex assertion (state change verification): ~15-20 minutes per iteration
- Integration assertion (multiple modules): ~20-30 minutes per iteration

## Related Patterns
- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md) - How to write proper RED phases
- [TDD GREEN Phase Minimalism](tdd-green-phase-minimalism.md) - Minimal implementation per iteration
- [TDD Plan Iteration Split Pattern](tdd-plan-iteration-split-pattern.md) - Splitting iterations to maintain one assertion per iteration
- [Temporary Test Cleanup Pattern](temporary-test-cleanup-pattern.md) - Managing memory in split iterations
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md) - Overall TDD cycle structure
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md) - The three phases
- [TDD Cycle Effort Estimation](tdd-cycle-effort-estimation.md) - Estimating TDD work
- [Requirement Precision in TDD](requirement-precision-in-tdd.md) - Breaking down requirements
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md) - Refining iteration plans
