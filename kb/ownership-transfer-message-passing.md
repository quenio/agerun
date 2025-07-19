# Ownership Transfer in Message Passing

## Learning
When agents create messages (like wake/sleep messages), they must explicitly mark themselves as owners using `ar_data__hold_ownership()`. The system must then check ownership and transfer it before destroying messages to prevent "Cannot destroy owned data" errors.

## Importance
This pattern prevents critical ownership violations that cause program crashes. Without proper ownership transfer, the system cannot destroy messages that agents have marked as owned, leading to either crashes or memory leaks.

## Example
```c
// From ar_agent.c - Agent marks itself as owner of wake message
ar_data_t *own_wake_msg = ar_data__create_string(g_wake_message);
if (own_wake_msg) {
    ar_data__hold_ownership(own_wake_msg, own_agent);  // Agent claims ownership
    ar_agent__send(own_agent, own_wake_msg);
}

// From ar_system.c - System transfers ownership before destruction
ar_data_t *own_message = ar_agent__get_message(ref_agent);
if (own_message) {
    // Check if message is owned by something
    static bool is_initialized = false;
    if (ar_data__hold_ownership(own_message, &is_initialized)) {
        // Transfer ownership to system before destroying
        ar_data__transfer_ownership(own_message, &is_initialized);
        ar_data__destroy(own_message);
    } else {
        // Not owned, can destroy directly
        ar_data__destroy(own_message);
    }
}

// From ar_send_instruction_evaluator.c - Handle unowned messages
if (agent_id == 0) {
    // For agent_id 0, message would be discarded
    // Must transfer ownership from evaluator before destroying
    ar_data__transfer_ownership(own_msg, evaluator);
    ar_data__destroy(own_msg);
    success = true;
}
```

## Generalization
The ownership transfer pattern applies to any scenario where:
1. One component creates data and marks itself as owner
2. Another component needs to destroy that data
3. Ownership must be explicitly transferred to allow destruction

Common scenarios:
- Agent wake/sleep messages
- Messages passed between agents
- Data returned from functions with ownership transfer
- Temporary values in evaluators

## Implementation
```c
// Pattern 1: Creator marks ownership
ar_data_t *own_data = ar_data__create_string("value");
ar_data__hold_ownership(own_data, owner_ptr);  // Mark as owned

// Pattern 2: Destroyer checks and transfers ownership from current owner
static bool dummy = false;
if (ar_data__hold_ownership(own_data, &dummy)) {
    ar_data__transfer_ownership(own_data, &dummy);  // Transfer from dummy owner
}
ar_data__destroy(own_data);

// Pattern 3: Release ownership by transferring from actual owner
void *actual_owner = ar_agent_ptr;  // The actual owner
ar_data__transfer_ownership(own_data, actual_owner);  // Transfer from owner releases
ar_data__destroy(own_data);
```

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Expression Ownership Rules](expression-ownership-rules.md)
- [Agent Wake Message Processing](agent-wake-message-processing.md)
- [Error Propagation Pattern](error-propagation-pattern.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)