# Message Ownership Flow in Agent-System Communication

## Problem
Messages sent to agents were becoming corrupted (MAP turning into INTEGER 0) due to improper ownership handling when messages were dequeued from agents. The unowned messages could be claimed by the expression evaluator, leading to premature destruction.

## Solution
The system must take ownership of messages immediately after dequeuing them from agents, ensuring proper ownership throughout the message lifecycle.

## Message Ownership Flow

```
1. Creation: Message created by sender (owns it)
2. Send: Agent takes ownership when message is queued
3. Dequeue: Agent drops ownership when message is removed
4. System: System takes ownership after dequeuing
5. Processing: System passes message as const reference to interpreter
6. Cleanup: System destroys message after processing
```

## Implementation

```c
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
bool ar_system__process_next_message_with_instance(ar_system_t *mut_system) {
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
```

## Key Insights

1. **Ownership Gap**: The critical issue was the gap between agent dropping ownership and system taking it
2. **Unowned Vulnerability**: Unowned messages can be claimed by any component (like expression evaluator)
3. **Const References**: Interpreter and frame store messages as const references, not taking ownership
4. **System Responsibility**: The system is responsible for message lifecycle after dequeuing

## Test Fixture Considerations

Test fixtures that bypass the normal system flow must replicate ownership management:

```c
// WRONG: Fixture calls interpreter directly without ownership
ar_interpreter__execute_method(interpreter, agent_id, message);  // Message unowned!

// CORRECT: Test fixture must manage ownership like system does
ar_data__take_ownership(message, fixture);
ar_interpreter__execute_method(interpreter, agent_id, message);
ar_data__destroy_if_owned(message, fixture);
```

**Critical**: Any code that calls `ar_interpreter__execute_method` directly must ensure the message has an owner to prevent the expression evaluator from claiming it during evaluation.

## Related Patterns
- [Ownership Drop in Message Passing](ownership-drop-message-passing.md) - Previous pattern (now updated)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Agent Wake Message Processing](agent-wake-message-processing.md)
- [Expression Ownership Rules](expression-ownership-rules.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [Ownership Gap Vulnerability](ownership-gap-vulnerability.md) - Detailed explanation of the gap issue
- [Debug Logging for Ownership Tracing](debug-logging-ownership-tracing.md) - How to debug these issues
- [Test Fixture Message Ownership](test-fixture-message-ownership.md) - Fixture-specific patterns