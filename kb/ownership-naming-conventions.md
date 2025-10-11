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
- **Zig modules**: Apply same prefixes to maintain consistency with C codebase
- **Local variables**: Even temporaries should use prefixes (e.g., `ref_method_name` not `method_name`)

## Wake/Sleep Message Ownership
When agents create wake or sleep messages, they must explicitly mark themselves as owners:
```c
// From ar_agent.c
ar_data_t *own_wake_msg = ar_data__create_string("__wake__");
if (own_wake_msg) {
    ar_data__take_ownership(own_wake_msg, own_agent);  // Agent claims ownership
    ar_agent__send(own_agent, own_wake_msg);
}
```

This ownership must be properly transferred when the system processes messages to avoid "Cannot destroy owned data" errors.

## Function Documentation
Document ownership semantics explicitly in function comments:

```c
/**
 * Get the proxy registry instance from a system
 * @param ref_system The system instance (borrowed reference)
 * @return The proxy registry instance (borrowed reference), or NULL if system is NULL
 * @note Ownership: Returns a borrowed reference - do not destroy
 */
ar_proxy_registry_t* ar_system__get_proxy_registry(const ar_system_t *ref_system);

/**
 * Register a proxy with the system
 * @param mut_system The system instance (mutable reference)
 * @param proxy_id The proxy ID (negative by convention)
 * @param own_proxy The proxy to register (ownership transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Takes ownership of own_proxy on success, caller must destroy on failure
 */
bool ar_system__register_proxy(ar_system_t *mut_system,
                                int64_t proxy_id,
                                ar_proxy_t *own_proxy);
```

**Ownership Documentation Patterns:**
- "Returns a borrowed reference - do not destroy"
- "Takes ownership of X on success, caller must destroy on failure"
- "Caller retains ownership - must destroy X after call"
- "Transfers ownership of X to Y"

## Related Patterns
- Memory Management Model (MMM)
- Zero memory leak tolerance
- Explicit resource management
- Debug assertions for ownership validation
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Context Ownership Lifecycle](context-ownership-lifecycle.md)