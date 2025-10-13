# TDD GREEN Phase Minimalism

**Note**: Examples use `ar_delegate` message queue functions (`send`, `has_messages`, `take_message`) which are planned for implementation in TDD Cycle 6.5, not yet implemented.  // EXAMPLE: TDD Cycle 6.5

## Learning
The GREEN phase should implement ONLY enough code to pass the specific assertion being tested, even if that means hardcoded return values. Later iterations force proper implementation by introducing new requirements. Sometimes you must "undo" or limit implementation from previous GREEN phases to ensure later RED phases properly fail.

## Importance
Minimal implementation reveals what code is actually needed to satisfy requirements. Hardcoded returns (like `return false;` or `return NULL;`) are valid GREEN implementations if they make the test pass. This discipline prevents over-engineering and ensures each piece of code has a test that demands its existence.

## Example

### Pattern 1: Hardcoded Returns Are Valid GREEN Implementation

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// Iteration 2 RED: has_messages() should return false when empty
static void test_delegate__has_no_messages_initially(void) {
    // Given a delegate with no messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When checking if delegate has messages
    bool has_messages = ar_delegate__has_messages(own_delegate);

    // Then it should return false
    AR_ASSERT(!has_messages, "Should have no messages initially");  // ← FAILS (stub returns true)

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}

// Iteration 2 GREEN: Minimal implementation
bool ar_delegate__has_messages(const ar_delegate_t *ref_delegate) {
    return false;  // Hardcoded! But it passes the test.
}
// This is CORRECT - don't implement queue checking yet

// Iteration 3 RED: has_messages() should return true after send()
// This iteration will FORCE proper implementation
```

### Pattern 2: Undoing Implementation to Force Proper RED

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// Iteration 5 GREEN: Return message from queue
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    if (!mut_delegate || !mut_delegate->own_message_queue) {
        return NULL;
    }
    // Use ar_list__first() to PEEK only (not remove)
    return (ar_data_t*)ar_list__first(mut_delegate->own_message_queue);
}
// Why peek instead of remove? So Iteration 6 can test removal!

// Iteration 6 RED: take_message() should remove from queue
static void test_delegate__take_message_removes_from_queue(void) {
    // Given a delegate with a message
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegate__send(own_delegate, ar_data__create_string("hello"));

    // When taking message and checking queue
    ar_data_t *own_message = ar_delegate__take_message(own_delegate);
    ar_data__destroy(own_message);
    bool has_messages = ar_delegate__has_messages(own_delegate);

    // Then queue should be empty
    AR_ASSERT(!has_messages, "Queue should be empty after take");  // ← FAILS (still in queue!)

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}

// Iteration 6 GREEN: Now implement removal
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    if (!mut_delegate || !mut_delegate->own_message_queue) {
        return NULL;
    }
    ar_data_t *own_msg = (ar_data_t*)ar_list__remove_first(mut_delegate->own_message_queue);
    if (own_msg) {
        ar_data__drop_ownership(own_msg, mut_delegate);
    }
    return own_msg;
}
```

## Generalization

### Ask "What's the Minimum?"
For each GREEN phase, ask:
1. **Can I return a hardcoded value?** If yes, do it.
2. **Can I use a simpler function?** (peek instead of remove)
3. **Can I skip error checking?** Let later iterations add it.
4. **Can I avoid creating infrastructure?** Wait until a test demands it.

### Common Minimal Implementations by Return Type
```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// Boolean functions
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (own_message) ar_data__destroy(own_message);
    return true;  // Hardcoded!
}

// Pointer functions
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    return NULL;  // Hardcoded!
}

// Integer functions
int64_t ar_delegate__count_messages(const ar_delegate_t *ref_delegate) {  // EXAMPLE: Hypothetical function
    return 0;  // Hardcoded!
}
```

### Progression Pattern
Watch how implementation grows iteration by iteration:

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// Iteration 1: Stub
bool ar_delegate__send(...) { return false; }

// Iteration 2: Hardcoded success
bool ar_delegate__send(...) {
    if (own_message) ar_data__destroy(own_message);
    return true;
}

// Iteration 3: Actual queuing (forced by has_messages() test)
bool ar_delegate__send(...) {
    ar_data__take_ownership(own_message, mut_delegate);
    return ar_list__add_last(mut_delegate->own_message_queue, own_message);
}

// Iteration 4: Error handling (forced by failure test)
bool ar_delegate__send(...) {
    if (!mut_delegate || !own_message) {
        if (own_message) ar_data__destroy(own_message);
        return false;
    }
    // ... queuing code ...
}
```

## Implementation

### Planning Checklist
When planning GREEN phase, verify:
- [ ] Implementation makes ONLY the new assertion pass
- [ ] No "future-proofing" or "while I'm here" additions
- [ ] Hardcoded returns considered and used if valid
- [ ] Implementation is reviewable in < 5 minutes

### Verification During Execution
```bash
# After GREEN implementation, verify minimalism:

# 1. Comment out the new implementation
# git diff modules/ar_delegate.c  # Should be small diff

# 2. Verify only the new test fails
make ar_delegate_tests 2>&1 | grep "FAILED"
# Should show only the test from current iteration

# 3. Check for accidental feature additions
git diff modules/ar_delegate.c | grep "^+" | wc -l
# Should be < 10 lines for simple assertions
```

### Common Over-Implementation Traps
1. **Adding error handling before test demands it**
   ```c  // EXAMPLE: TDD Cycle 6.5 planned functions
   // TOO MUCH for early iteration
   if (!mut_delegate || !own_message || !mut_delegate->own_message_queue) {
       // Tests don't check this yet!
   }
   ```

2. **Implementing full feature when partial works**
   ```c  // EXAMPLE: TDD Cycle 6.5 planned functions
   // If test only checks return value, don't queue yet!
   bool ar_delegate__send(...) {
       ar_data__destroy(own_message);
       return true;  // Sufficient for Iteration 1
   }
   ```

3. **Optimizing before test demands it**
   ```c
   // Don't add caching, batching, or optimization until tests require it
   ```

## Related Patterns
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md) - Planning one assertion per iteration
- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md) - Proper RED phase execution
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md) - The complete TDD cycle
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md) - TDD methodology overview
- [Requirement Precision in TDD](requirement-precision-in-tdd.md) - Breaking down requirements precisely
