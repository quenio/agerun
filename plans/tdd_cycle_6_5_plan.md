# TDD Cycle 6.5: Add Message Queue Infrastructure to ar_delegation

**Note**: This is an implementation plan. All function references (`ar_delegate__send`, `ar_delegate__has_messages`, `ar_delegate__take_message`, `ar_delegation__send_to_delegate`, `ar_delegation__delegate_has_messages`, `ar_delegation__take_delegate_message`) are planned implementations, not yet created. This plan documents the TDD methodology for creating these functions.

## Overview
Implement message queuing for delegates with proper ownership semantics. Each iteration has ONE assertion, GREEN implements ONLY enough to pass that assertion. All tests use BDD structure.

**Ownership Flow**: Delegate takes ownership when queuing → Delegate owns while queued → Delegate drops ownership when taking → **Caller MUST destroy returned message**

---

## Iteration 1: Send returns success

### RED
- Add declaration: `bool ar_delegate__send(ar_delegate_t*, ar_data_t*)` to ar_delegate.h
- Add stub returning `false` to ar_delegate.c
- Write test:
```c
static void test_delegate__send_returns_true(void) {
    // Given a delegate instance
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When sending a message
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegate__send(own_delegate, own_message);

    // Then send should return true
    AR_ASSERT(result, "Send should return true");  // ← FAILS (stub returns false)

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Minimal implementation:
```c
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (!mut_delegate || !own_message) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    ar_data__destroy(own_message);  // Destroy for now (no queue yet)
    return true;
}
```

### REFACTOR
- Clean up, verify no leaks

---

## Iteration 2: Has messages returns false when empty

### RED
- Add declaration: `bool ar_delegate__has_messages(const ar_delegate_t*)` to ar_delegate.h
- Add stub returning `true` to ar_delegate.c (intentionally wrong)
- Write test:
```c
static void test_delegate__has_no_messages_initially(void) {
    // Given a delegate instance with no messages sent
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
```
- **Expected:** Assertion fails

### GREEN
- Minimal implementation: `return false;`

### REFACTOR
- Clean up

---

## Iteration 3: Has messages returns true after send

### RED
- Write test:
```c
static void test_delegate__has_messages_after_send(void) {
    // Given a delegate with a sent message
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When sending a message and checking if delegate has messages
    ar_delegate__send(own_delegate, ar_data__create_string("hello"));
    bool has_messages = ar_delegate__has_messages(own_delegate);

    // Then it should return true
    AR_ASSERT(has_messages, "Should have messages after send");  // ← FAILS (returns false)

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Implement queuing infrastructure:
  - Add `ar_list_t *own_message_queue` field to struct ar_delegate_s
  - Initialize in `ar_delegate__create()`: `own_message_queue = ar_list__create()`
  - Update `ar_delegate__send()`:
    ```c
    bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
        if (!mut_delegate || !own_message) {
            if (own_message) ar_data__destroy(own_message);
            return false;
        }
        if (!mut_delegate->own_message_queue) {
            ar_data__destroy_if_owned(own_message, mut_delegate);
            return false;
        }
        // Take ownership for the delegate before adding to queue
        ar_data__take_ownership(own_message, mut_delegate);
        bool result = ar_list__add_last(mut_delegate->own_message_queue, own_message);
        if (!result) {
            ar_data__destroy_if_owned(own_message, mut_delegate);
        }
        return result;
    }
    ```
  - Update `ar_delegate__has_messages()`: `return ar_list__count(queue) > 0;`
  - Update `ar_delegate__destroy()` (pattern: ar_agent.c:78-89):
    ```c
    if (own_delegate->own_message_queue) {
        ar_data_t *own_msg = NULL;
        while ((own_msg = ar_list__remove_first(own_delegate->own_message_queue)) != NULL) {
            ar_data__destroy_if_owned(own_msg, own_delegate);
        }
        ar_list__destroy(own_delegate->own_message_queue);
    }
    ```

### REFACTOR
- Add includes: ar_list.h, ar_data.h
- Verify ownership comments
- Run tests, verify zero leaks

---

## Iteration 4: Take message returns NULL when empty

### RED
- Add declaration: `ar_data_t* ar_delegate__take_message(ar_delegate_t*)` to ar_delegate.h
- Add stub returning `ar_data__create_string("fake")` to ar_delegate.c
- Write test:
```c
static void test_delegate__take_message_returns_null_when_empty(void) {
    // Given a delegate with no messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When taking a message
    ar_data_t *own_message = ar_delegate__take_message(own_delegate);

    // Then it should return NULL
    AR_ASSERT(own_message == NULL, "Should return NULL when empty");  // ← FAILS (stub returns fake)

    // Cleanup (no message to destroy - returned NULL)
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Minimal implementation: `return NULL;`

### REFACTOR
- Clean up

---

## Iteration 5: Take message returns queued message

### RED
- Write test:
```c
static void test_delegate__take_message_returns_sent_message(void) {
    // Given a delegate with a sent message
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegate__send(own_delegate, ar_data__create_string("hello"));

    // When taking a message
    ar_data_t *own_received = ar_delegate__take_message(own_delegate);

    // Then it should return the message
    AR_ASSERT(own_received != NULL, "Should return message");  // ← FAILS (returns NULL)

    // Cleanup - MUST destroy message (delegate dropped ownership)
    ar_data__destroy(own_received);
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Peek without removing:
```c
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    if (!mut_delegate || !mut_delegate->own_message_queue) {
        return NULL;
    }
    return (ar_data_t*)ar_list__first(mut_delegate->own_message_queue);
}
```

### REFACTOR
- Run tests, verify it passes

---

## Iteration 6: Take message removes message from queue

### RED
- **Note: Current implementation peeks with ar_list__first()**
- Write test:
```c
static void test_delegate__take_message_removes_from_queue(void) {
    // Given a delegate with a sent message
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegate__send(own_delegate, ar_data__create_string("hello"));

    // When taking the message and checking queue
    ar_data_t *own_message = ar_delegate__take_message(own_delegate);
    ar_data__destroy(own_message);  // MUST destroy (delegate dropped ownership)
    bool has_messages = ar_delegate__has_messages(own_delegate);

    // Then the queue should be empty
    AR_ASSERT(!has_messages, "Queue should be empty after take");  // ← FAILS (still in queue)

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Remove and drop ownership (pattern: ar_agent.c:134-145):
```c
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {
    if (!mut_delegate || !mut_delegate->own_message_queue) {
        return NULL;
    }
    ar_data_t *own_msg = (ar_data_t*)ar_list__remove_first(mut_delegate->own_message_queue);
    if (own_msg) {
        // Drop ownership - caller must destroy
        ar_data__drop_ownership(own_msg, mut_delegate);
    }
    return own_msg;
}
```

### REFACTOR
- Verify ownership transfer
- Run tests, verify zero leaks

---

## Iteration 7: Send to delegate via delegation returns true

### RED
- Add declaration: `bool ar_delegation__send_to_delegate(ar_delegation_t*, int64_t, ar_data_t*)` to ar_delegation.h
- Add stub returning `false` to ar_delegation.c
- Write test:
```c
static void test_delegation__send_to_delegate_returns_true(void) {
    // Given a delegation with a registered delegate
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending a message to the delegate
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegation__send_to_delegate(own_delegation, -100, own_message);

    // Then send should return true
    AR_ASSERT(result, "Should return true");  // ← FAILS (stub returns false)

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Minimal implementation:
```c
bool ar_delegation__send_to_delegate(ar_delegation_t *mut_delegation, int64_t delegate_id, ar_data_t *own_message) {
    (void)mut_delegation;
    (void)delegate_id;
    if (own_message) {
        ar_data__destroy(own_message);
    }
    return true;
}
```

### REFACTOR
- Clean up

---

## Iteration 8: Send to delegate actually queues message

### RED
- Write test:
```c
static void test_delegation__send_actually_queues_message(void) {
    // Given a delegation with a registered delegate
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending a message via delegation and checking if queued
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("hello"));
    ar_delegate_registry_t *ref_registry = ar_delegation__get_registry(own_delegation);
    ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);
    bool has = ar_delegate__has_messages(ref_found);

    // Then the delegate should have the message queued
    AR_ASSERT(has, "Delegate should have queued message");  // ← FAILS (message destroyed)

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Implement properly (pattern: ar_agency.c:258-302):
```c
bool ar_delegation__send_to_delegate(ar_delegation_t *mut_delegation, int64_t delegate_id, ar_data_t *own_message) {
    if (!mut_delegation || !mut_delegation->own_registry) {
        if (own_message) ar_data__destroy(own_message);
        return false;
    }
    ar_delegate_t *mut_delegate = ar_delegate_registry__find(mut_delegation->own_registry, delegate_id);
    if (!mut_delegate) {
        if (own_message) ar_data__destroy(own_message);
        return false;
    }
    return ar_delegate__send(mut_delegate, own_message);
}
```

### REFACTOR
- Run tests, verify zero leaks

---

## Iteration 9: Delegation has messages check returns false initially

### RED
- Add declaration: `bool ar_delegation__delegate_has_messages(ar_delegation_t*, int64_t)` to ar_delegation.h
- Add stub returning `true` to ar_delegation.c
- Write test:
```c
static void test_delegation__delegate_has_no_messages_initially(void) {
    // Given a delegation with a registered delegate (no messages sent)
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When checking if delegate has messages
    bool has = ar_delegation__delegate_has_messages(own_delegation, -100);

    // Then it should return false
    AR_ASSERT(!has, "Should have no messages initially");  // ← FAILS (stub returns true)

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Minimal: `return false;`

### REFACTOR
- Clean up

---

## Iteration 10: Delegation has messages check returns true after send

### RED
- Write test:
```c
static void test_delegation__delegate_has_messages_after_send(void) {
    // Given a delegation with a delegate that has received a message
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending a message and checking if delegate has messages
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("hello"));
    bool has = ar_delegation__delegate_has_messages(own_delegation, -100);

    // Then it should return true
    AR_ASSERT(has, "Should have messages after send");  // ← FAILS (returns false)

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Implement (pattern: ar_agency.c:136-147):
```c
bool ar_delegation__delegate_has_messages(ar_delegation_t *ref_delegation, int64_t delegate_id) {
    if (!ref_delegation || !ref_delegation->own_registry) {
        return false;
    }
    ar_delegate_t *ref_delegate = ar_delegate_registry__find(ref_delegation->own_registry, delegate_id);
    if (!ref_delegate) {
        return false;
    }
    return ar_delegate__has_messages(ref_delegate);
}
```

### REFACTOR
- Run tests

---

## Iteration 11: Take delegate message returns NULL when empty

### RED
- Add declaration: `ar_data_t* ar_delegation__take_delegate_message(ar_delegation_t*, int64_t)` to ar_delegation.h
- Add stub returning `ar_data__create_string("fake")` to ar_delegation.c
- Write test:
```c
static void test_delegation__take_delegate_message_null_when_empty(void) {
    // Given a delegation with a delegate that has no messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When taking a message from the delegate
    ar_data_t *own_msg = ar_delegation__take_delegate_message(own_delegation, -100);

    // Then it should return NULL
    AR_ASSERT(own_msg == NULL, "Should return NULL when empty");  // ← FAILS (stub returns fake)

    // Cleanup (no message to destroy - returned NULL)
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Minimal: `return NULL;`

### REFACTOR
- Clean up

---

## Iteration 12: Take delegate message returns queued message

### RED
- Write test:
```c
static void test_delegation__take_delegate_message_returns_message(void) {
    // Given a delegation with a delegate that has a queued message
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("hello"));

    // When taking a message from the delegate
    ar_data_t *own_msg = ar_delegation__take_delegate_message(own_delegation, -100);

    // Then it should return the message
    AR_ASSERT(own_msg != NULL, "Should return message");  // ← FAILS (returns NULL)

    // Cleanup - MUST destroy message (delegate dropped ownership)
    ar_data__destroy(own_msg);
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Assertion fails

### GREEN
- Implement (pattern: ar_agency.c:149-160):
```c
ar_data_t* ar_delegation__take_delegate_message(ar_delegation_t *mut_delegation, int64_t delegate_id) {
    if (!mut_delegation || !mut_delegation->own_registry) {
        return NULL;
    }
    ar_delegate_t *mut_delegate = ar_delegate_registry__find(mut_delegation->own_registry, delegate_id);
    if (!mut_delegate) {
        return NULL;
    }
    return ar_delegate__take_message(mut_delegate);
}
```

### REFACTOR
- Run tests, verify zero leaks

---

## Iteration 13: Send to non-existent delegate returns false

### RED
- Write test:
```c
static void test_delegation__send_to_nonexistent_returns_false(void) {
    // Given a delegation with no registered delegates
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);

    // When sending to a non-existent delegate
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegation__send_to_delegate(own_delegation, -999, own_message);

    // Then it should return false
    AR_ASSERT(!result, "Should return false for non-existent delegate");  // ← Should pass from Iteration 8

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}
```
- **Expected:** Should already pass

### GREEN
- Already implemented

### REFACTOR
- Verify message destroyed (no leaks)

---

## Iteration 14: Destroy delegate with queued messages (no leaks)

### RED
- Write test:
```c
static void test_delegation__destroy_with_queued_messages(void) {
    // Given a delegation with a delegate that has queued messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending messages and destroying the delegation
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("msg1"));
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("msg2"));
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);

    // Then memory report should show zero leaks
}
```
- **Expected:** Should pass from Iteration 3

### GREEN
- Already implemented

### REFACTOR
- Run: `make ar_delegation_tests 2>&1`
- Verify: `grep "Actual memory leaks: 0" bin/run-tests/memory_report_ar_delegation_tests.log`

---

## Final Tasks

**Documentation:**
- Update ar_delegate.md: Add "Message Queue" section
- Document ownership flow: delegate takes → delegate owns → delegate drops → **caller MUST destroy**

**Testing:**
- Run full suite: `make ar_delegation_tests 2>&1`
- Verify all memory reports show zero leaks

---

## Files Modified
- modules/ar_delegate.h (3 declarations)
- modules/ar_delegate.c (queue field + 3 implementations)
- modules/ar_delegation.h (3 declarations)
- modules/ar_delegation.c (3 implementations)
- modules/ar_delegate_tests.c (6 tests with BDD)
- modules/ar_delegation_tests.c (8 tests with BDD)
- modules/ar_delegate.md (Message Queue section)

## Success Criteria
✅ Proper ownership: take_ownership when queuing, drop_ownership when taking
✅ **Caller destroys all non-NULL messages from take_message/take_delegate_message**
✅ Cleanup uses ar_data__destroy_if_owned()
✅ Each RED has explicit failing assertion
✅ Each GREEN implements minimum
✅ BDD structure (Given/When/Then/Cleanup)
✅ Message creation in "When"
✅ Clear function names (take_message)
✅ Zero memory leaks

## Structure
**TDD Cycle 6.5:** 14 iterations
