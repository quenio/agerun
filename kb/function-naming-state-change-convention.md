# Function Naming State Change Convention

**Note**: Examples use hypothetical `ar_delegate` functions to demonstrate naming conventions. Functions marked with `// EXAMPLE:` are teaching examples, not existing code.

## Learning
Use "take_" prefix for operations that remove items and transfer ownership to the caller. Use "get_" prefix for read-only operations that return attributes or borrowed references. Use "peek_" prefix for read-only access to queued items without removal. Function names should signal their side effects.

## Importance
Function names communicate intent and side effects without requiring documentation review. "get" implies idempotent read-only access; "take" implies destructive read with ownership transfer. Clear naming prevents misuse and makes ownership transfer obvious at call sites.

## Example

### WRONG: Using "get" for State-Changing Operation
```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// BAD: Function name suggests read-only, but actually removes message
ar_data_t* ar_delegate__get_message(ar_delegate_t *mut_delegate) {  // EXAMPLE: Bad naming pattern
    ar_data_t *own_msg = ar_list__remove_first(mut_delegate->own_message_queue);
    if (own_msg) {
        ar_data__drop_ownership(own_msg, mut_delegate);
    }
    return own_msg;
}

// Misleading usage - looks like idempotent read
ar_data_t *msg1 = ar_delegate__get_message(own_delegate);  // Returns message
ar_data_t *msg2 = ar_delegate__get_message(own_delegate);  // Returns different message!
// Surprised caller: "I just got the message, why is it different now?"
```

### CORRECT: Using "take" for State-Changing Operation
```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// GOOD: Function name signals removal and ownership transfer
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {  // EXAMPLE: Good naming pattern to implement
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

// Clear usage - obviously destructive
ar_data_t *own_msg = ar_delegate__take_message(mut_delegate);
if (own_msg) {
    // ... process message ...
    ar_data__destroy(own_msg);  // Obviously caller's responsibility
}
```

### Pattern Comparison: get vs take vs peek

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// PATTERN 1: get_ - Read-only attribute access
int64_t ar_delegate__get_id(const ar_delegate_t *ref_delegate) {  // EXAMPLE: Hypothetical getter
    if (!ref_delegate) return 0;
    return ref_delegate->id;  // No state change, can call repeatedly
}
// Usage: Idempotent
int64_t id1 = ar_delegate__get_id(ref_delegate);
int64_t id2 = ar_delegate__get_id(ref_delegate);
// id1 == id2 always

// PATTERN 2: peek_ - Read-only queue access without removal
ar_data_t* ar_delegate__peek_message(const ar_delegate_t *ref_delegate) {  // EXAMPLE: Hypothetical peek function
    if (!ref_delegate || !ref_delegate->own_message_queue) {
        return NULL;
    }
    return (ar_data_t*)ar_list__first(ref_delegate->own_message_queue);
}
// Usage: Idempotent, returns borrowed reference
ar_data_t *ref_msg1 = ar_delegate__peek_message(ref_delegate);  // Returns first message
ar_data_t *ref_msg2 = ar_delegate__peek_message(ref_delegate);  // Returns same message
// ref_msg1 == ref_msg2, do NOT destroy (borrowed)

// PATTERN 3: take_ - Destructive read with ownership transfer
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate) {  // EXAMPLE: To be implemented
    if (!mut_delegate || !mut_delegate->own_message_queue) {
        return NULL;
    }
    ar_data_t *own_msg = (ar_data_t*)ar_list__remove_first(mut_delegate->own_message_queue);
    if (own_msg) {
        ar_data__drop_ownership(own_msg, mut_delegate);
    }
    return own_msg;
}
// Usage: Destructive, returns owned value
ar_data_t *own_msg1 = ar_delegate__take_message(mut_delegate);  // Removes first
ar_data_t *own_msg2 = ar_delegate__take_message(mut_delegate);  // Removes next
// own_msg1 != own_msg2 (different messages), MUST destroy both
ar_data__destroy(own_msg1);
ar_data__destroy(own_msg2);
```

## Generalization

### Naming Decision Tree

```
Does the function modify object state?
├─ NO → Does it return a value from the object?
│       ├─ YES → Use "get_" prefix
│       │        Example: ar_delegate__get_id()  // EXAMPLE: Hypothetical
│       └─ NO → Use descriptive verb
│                Example: ar_delegate__has_messages()  // EXAMPLE: To be implemented
└─ YES → Does it remove/extract a value?
          ├─ YES → Use "take_" prefix (implies ownership transfer)
          │        Example: ar_delegate__take_message()  // EXAMPLE: To be implemented
          └─ NO → Use specific action verb
                   Example: ar_delegate__send()  // EXAMPLE: To be implemented
                   Example: ar_delegate__clear_messages()  // EXAMPLE: Hypothetical
```

### Convention Rules

| Prefix | State Change | Ownership | Idempotent | Example |
|--------|-------------|-----------|------------|---------|
| `get_` | No | Borrowed ref | Yes | `ar_delegate__get_id()` |  // EXAMPLE: TDD Cycle 6.5
| `peek_` | No | Borrowed ref | Yes | `ar_delegate__peek_message()` |  // EXAMPLE: TDD Cycle 6.5
| `take_` | Yes (removes) | Transfers | No | `ar_delegate__take_message()` |  // EXAMPLE: TDD Cycle 6.5
| `has_` | No | N/A | Yes | `ar_delegate__has_messages()` |  // EXAMPLE: TDD Cycle 6.5
| `is_` | No | N/A | Yes | `ar_delegate__is_initialized()` |  // EXAMPLE: TDD Cycle 6.5
| `set_` | Yes (mutates) | N/A | No | `ar_delegate__set_name()` |  // EXAMPLE: TDD Cycle 6.5
| `add_` | Yes (adds) | Takes ownership | No | `ar_list__add_last()` |
| `remove_` | Yes (removes) | Transfers | No | `ar_list__remove_first()` |

### Existing AgeRun Patterns

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// get_ - Attribute access (borrowed)
ar_log_t* ar_system__get_log(const ar_system_t *ref_system);  // Real function
ar_data_type_t ar_data__get_type(const ar_data_t *ref_data);  // Real function

// take_ - Ownership transfer (would be good names for future refactoring)
// Currently named "get_message" but should be "take_message":
ar_data_t* ar_agent__get_message(ar_agent_t *mut_agent);  // Real function - transfers ownership

// has_ - Boolean check
bool ar_agent__has_messages(const ar_agent_t *ref_agent);  // Real function

// is_ - State check
bool ar_data__is_owned(const ar_data_t *ref_data, const void *ref_owner);  // Real function

// send_ - Action with ownership transfer
bool ar_agent__send(ar_agent_t *mut_agent, ar_data_t *own_message);  // Real function
```

## Implementation

### Refactoring Checklist for Existing Code

When encountering `get_` functions that change state:

```bash
# 1. Identify state-changing get_ functions
grep -n "get_message\|get_next\|get_and_remove" modules/*.h

# 2. Check if they transfer ownership
grep -A 10 "function_name" modules/*.c | grep "drop_ownership"

# 3. If yes, consider renaming to take_
# Note: This would be a breaking API change requiring systematic updates

# 4. Document current behavior clearly
# Add @note comments about ownership transfer and state change
```

### For New Code

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// When designing new functions, choose name based on behavior:

// Read-only attribute? Use get_
const char* ar_delegate__get_name(const ar_delegate_t *ref_delegate);  // EXAMPLE: Hypothetical

// Read-only queue peek? Use peek_
ar_data_t* ar_delegate__peek_next_message(const ar_delegate_t *ref_delegate);  // EXAMPLE: Hypothetical

// Remove and transfer ownership? Use take_
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate);  // EXAMPLE: To be implemented

// Add and take ownership? Use add_ or send_
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message);  // EXAMPLE: To be implemented

// Boolean check? Use has_ or is_
bool ar_delegate__has_messages(const ar_delegate_t *ref_delegate);  // EXAMPLE: To be implemented
bool ar_delegate__is_idle(const ar_delegate_t *ref_delegate);  // EXAMPLE: Hypothetical
```

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md) - Variable prefix conventions
- [API Ownership on Failure](api-ownership-on-failure.md) - Ownership in error cases
- [Message Ownership Flow](message-ownership-flow.md) - Complete ownership lifecycle
- [Context Ownership Lifecycle](context-ownership-lifecycle.md) - Ownership patterns
- [Expression Ownership Rules](expression-ownership-rules.md) - Ownership in evaluators
