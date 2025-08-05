# Defensive Programming Consistency

## Learning
When working with ownership and memory management, consistency in API usage is more important than micro-optimizations. Use defensive functions even when you "know" the ownership state.

## Importance
Consistent API usage reduces cognitive load, prevents future bugs when assumptions change, and makes code reviews easier. The performance cost of defensive checks is negligible compared to the cost of debugging memory issues.

## Example
In evaluators, even when we know data is owned after `ar_data__claim_or_copy()`:
```c
// Less consistent approach - mixes defensive and direct calls
ar_data_t *own_message = ar_data__claim_or_copy(message, evaluator);
if (agent_id == 0) {
    // We "know" own_message is owned, but this breaks consistency
    ar_data__destroy(own_message);  // BAD: Inconsistent API usage
}

// Consistent defensive approach
ar_data_t *own_message = ar_data__claim_or_copy(message, evaluator);
if (agent_id == 0) {
    // Always use defensive cleanup
    ar_data__destroy_if_owned(own_message, evaluator);  // Good: Consistent
}
```

## Generalization
Apply defensive programming consistently:
1. Choose defensive APIs when available (e.g., `ar_data__destroy_if_owned()`)
2. Use them everywhere, not just where "necessary"
3. Document when you're being defensive vs. when ownership is guaranteed
4. Prefer consistency over micro-optimizations
5. Make the safe path the easy path

## Implementation
When refactoring to use defensive patterns:
```c
// Search for all destroy patterns after claim_or_copy
grep -A5 "claim_or_copy" modules/*.c | grep -B5 "destroy"

// Replace direct destroy with defensive version
// Even in cases where ownership is "guaranteed"
ar_data__destroy_if_owned(data, owner);

// This consistency helps when code evolves
// Future changes won't introduce subtle bugs
```

### Message Type Handling
Defensive programming is especially important when dealing with mixed message types:

```c
// Non-defensive: Assumes message is always a map
ar_data_t* ref_sender = ar_data__get_map_data(message, "sender");  // BAD: Crashes on string messages

// Defensive: Handle both string and map messages
ar_data_type_t msg_type = ar_data__get_type(message);
ar_data_t* own_sender;
if (msg_type == AR_DATA_TYPE_STRING) {
    // Handle special messages like __wake__
    own_sender = ar_data__create_integer(0);  // Default sender
} else if (msg_type == AR_DATA_TYPE_MAP) {
    // Safe to access map fields
    ar_data_t* ref_sender_field = ar_data__get_map_data(message, "sender");
    own_sender = ref_sender_field ? ar_data__shallow_copy(ref_sender_field) : ar_data__create_integer(0);
} else {
    // Handle other types defensively
    own_sender = ar_data__create_integer(0);
}
```

In AgeRun method language, use defensive value selection:
```
memory.is_special := if(message = "__wake__", 1, 0)
memory.sender := if(memory.is_special = 1, 0, message.sender)  # Defensive default
```

## Related Patterns
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Wake Message Field Access Pattern](wake-message-field-access-pattern.md)