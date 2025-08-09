# Duplicate Wake Message Bug

## Learning
The ar_system__init_with_instance function sends a wake message to newly created agents even though agents automatically send themselves a wake message upon creation. This results in duplicate wake messages being queued.

## Importance
This architectural bug causes unexpected behavior in message processing loops, where one "extra" message is always processed. Understanding this bug is crucial for writing accurate tests and avoiding confusion about message counts.

## Example
```c
// Current buggy implementation in ar_system.c:
int64_t initial_agent = ar_agency__create_agent_with_instance(
    mut_system->own_agency, ref_method_name, ref_version, NULL);

if (initial_agent != 0) {
    // BUG: Agent already sent itself a wake message during creation
    own_wake_data = ar_data__create_string("__wake__");
    if (own_wake_data) {
        ar_agency__send_to_agent_with_instance(mut_system->own_agency, 
                                              initial_agent, 
                                              own_wake_data);
        // Process one wake message (leaving duplicate in queue)
        ar_system__process_next_message_with_instance(mut_system);
    }
}

// Result: Agent has TWO wake messages but only one is processed
// The duplicate remains in queue for later processing
```

## Generalization
When components have auto-initialization behavior (like agents sending wake messages to themselves), manual initialization from higher-level components creates duplication. This pattern of duplicate initialization can occur whenever:
- Lower-level components handle their own lifecycle events
- Higher-level components aren't aware of auto-initialization
- There's no coordination between initialization layers

## Implementation
To work around this bug in tests:
```c
// Test expects exactly 1 message (the duplicate wake)
int messages_processed = ar_system__process_all_messages_with_instance(mut_system);
AR_ASSERT(messages_processed == 1, "Should process 1 message (duplicate wake)");

// Document the bug in comments
// Note: Currently processes duplicate wake message (bug: ar_system__init sends extra wake)
```

To fix the bug properly:
```c
// Remove the duplicate wake send from ar_system__init_with_instance
// Let agents handle their own wake messages
int64_t initial_agent = ar_agency__create_agent_with_instance(
    mut_system->own_agency, ref_method_name, ref_version, NULL);

// Just process the auto-sent wake message
if (initial_agent != 0) {
    ar_system__process_next_message_with_instance(mut_system);
}
```

## Related Patterns
- [Agent Wake Message Processing](agent-wake-message-processing.md)
- [Message Ownership Flow](message-ownership-flow.md)
- [Test Expectation Reality Alignment](test-expectation-reality-alignment.md)