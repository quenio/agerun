# Ownership Naming Conventions

## Learning
Use ownership prefixes (own_, mut_, ref_) for ALL variables, parameters, and struct fields to make memory management explicit

## Importance
Prevents memory leaks and use-after-free bugs by making ownership clear at the code level

## Example
```c
// Clear ownership semantics
ar_data_t *own_result = ar_data__create_string("hello");  // Must destroy
ar_data_t *ref_item = ar_data__get_map_value(map, "key"); // Borrowed reference  // EXAMPLE: Hypothetical function
ar_data_t *mut_memory = context->memory;                  // Mutable access

// Ownership transfer
ar_data__destroy(own_result);
own_result = NULL;  // Clear after transfer
```

## Generalization
Make ownership and lifetime explicit in variable names rather than relying on comments or documentation

## Implementation
- `own_`: Owned values that must be destroyed by this code
- `mut_`: Mutable references (read-write access, not owned)
- `ref_`: Borrowed references (read-only access, not owned)
- Apply consistently across .h, .c, and _tests.c files
- Use immediately when receiving ownership from functions

## Wake/Sleep Message Ownership
When agents create wake or sleep messages, they must explicitly mark themselves as owners:
```c
// From ar_agent.c
ar_data_t *own_wake_msg = ar_data__create_string("__wake__");
if (own_wake_msg) {
    ar_data__hold_ownership(own_wake_msg, own_agent);  // Agent claims ownership
    ar_agent__send(own_agent, own_wake_msg);
}
```

This ownership must be properly transferred when the system processes messages to avoid "Cannot destroy owned data" errors.

## Related Patterns
- Memory Management Model (MMM)
- Zero memory leak tolerance
- Explicit resource management
- Debug assertions for ownership validation
- [Ownership Transfer in Message Passing](ownership-transfer-message-passing.md)
- [Agent Wake Message Processing](agent-wake-message-processing.md)