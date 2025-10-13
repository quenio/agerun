# Message Ownership Flow in Message Queue Operations

**Note**: Examples include `ar_delegate` message queue functions (`send`, `take_message`) which are planned for implementation in TDD Cycle 6.5, not yet implemented.

## Problem
Messages in queue operations require careful ownership management. When messages are dequeued, ownership must be properly transferred to prevent corruption or memory leaks. This pattern applies to both agent-system communication and delegate message queuing.

## Solution
The owner (agent, delegate, or system) must take ownership when queuing messages and drop ownership when dequeuing. The caller must take responsibility for destroying dequeued messages.

## Message Ownership Flow

### Universal Pattern (applies to agents, delegates, and systems)
```
1. Creation: Message created by sender (owns it)
2. Send/Queue: Owner takes ownership when message is queued
3. Dequeue: Owner drops ownership when message is removed
4. Caller: Caller MUST destroy the returned message
```

### Agent-System Communication Flow
```
1. Creation: Message created by sender (owns it)
2. Send: Agent takes ownership when message is queued
3. Dequeue: Agent drops ownership when message is removed
4. System: System takes ownership after dequeuing
5. Processing: System passes message as const reference to interpreter
6. Cleanup: System destroys message after processing
```

### Delegate Message Queuing Flow
```
1. Creation: Message created by sender (owns it)
2. Send: Delegate takes ownership when message is queued
3. Take: Delegate drops ownership when message is removed
4. Caller: Caller MUST destroy the returned message
```

## Implementation

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// 1. Agent takes ownership when receiving message (ar_agent.c)
bool ar_agent__send(ar_agent_t *mut_agent, ar_data_t *own_message) {
    // Take ownership for the agent before adding to queue
    ar_data__take_ownership(own_message, mut_agent);
    
    // Agent module adds to the queue
    bool result = ar_list__add_last(mut_agent->own_message_queue, own_message);
    // ...
}

// 2. Agent drops ownership when dequeuing (ar_agent.c)
ar_data_t* ar_agent__get_message(ar_agent_t *mut_agent) {
    ar_data_t *own_message = ar_list__remove_first(mut_agent->own_message_queue);
    if (own_message) {
        // Transfer ownership from agent to NULL (making it unowned)
        ar_data__drop_ownership(own_message, mut_agent);
    }
    return own_message;
}

// 3. System takes ownership after dequeuing (ar_system.c)
bool ar_system__process_next_message(ar_system_t *mut_system) {
    // ... get message from agent ...

    if (own_message) {
        // Take ownership of the message for the system
        ar_data__take_ownership(own_message, mut_system);

        ar_interpreter__execute_method(mut_system->own_interpreter, agent_id, own_message);

        // Free the message as it's now been processed
        ar_data__destroy_if_owned(own_message, mut_system);
        own_message = NULL;
        return true;
    }
    // ...
}

// DELEGATE EXAMPLE: Same pattern applies to delegates

// 1. Delegate takes ownership when receiving message (ar_delegate.c)
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (!mut_delegate || !own_message) {
        if (own_message) ar_data__destroy(own_message);
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

// 2. Delegate drops ownership when dequeuing (ar_delegate.c)
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

// 3. Caller takes responsibility for destroying (test or application code)
ar_data_t *own_message = ar_delegate__take_message(mut_delegate);
if (own_message) {
    // ... process message ...
    ar_data__destroy(own_message);  // MANDATORY: caller owns it now
}
```

## Key Insights

1. **Universal Pattern**: The take-ownership/drop-ownership pattern applies uniformly to agents, delegates, and systems
2. **Caller Responsibility**: After dequeuing, the caller MUST destroy the message (ownership has been dropped)
3. **Test Message Cleanup**: Tests that call take_message functions must destroy returned messages to prevent leaks
4. **Ownership Gap**: Unowned messages can be claimed by any component (like expression evaluator)
5. **Const References**: Components that don't own messages store them as const references only
6. **Function Naming**: Functions that transfer ownership should use "take_" prefix, not "get_"

## Test Fixture Considerations

Test fixtures that bypass the normal system flow must replicate ownership management:

```c  // EXAMPLE: TDD Cycle 6.5 planned functions
// WRONG: Fixture calls interpreter directly without ownership
ar_interpreter__execute_method(interpreter, agent_id, message);  // Message unowned!

// CORRECT: Test fixture must manage ownership like system does
ar_data__take_ownership(message, fixture);
ar_interpreter__execute_method(interpreter, agent_id, message);
ar_data__destroy_if_owned(message, fixture);
```

**Critical**: Any code that calls `ar_interpreter__execute_method` directly must ensure the message has an owner to prevent the expression evaluator from claiming it during evaluation.

## Related Patterns
- [Function Naming State Change Convention](function-naming-state-change-convention.md) - take_ vs get_ naming
- [BDD Test Structure](bdd-test-structure.md) - Proper test organization including cleanup
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Expression Ownership Rules](expression-ownership-rules.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [Ownership Gap Vulnerability](ownership-gap-vulnerability.md) - Detailed explanation of the gap issue
- [Debug Logging for Ownership Tracing](debug-logging-ownership-tracing.md) - How to debug these issues
- [Test Fixture Message Ownership](test-fixture-message-ownership.md) - Fixture-specific patterns