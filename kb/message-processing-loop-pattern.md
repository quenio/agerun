# Message Processing Loop Pattern

## Learning
Message processing in AgeRun requires a loop that continues until all messages in all agent queues are processed. The ar_system__process_all_messages_with_instance() function provides this capability, returning the count of messages processed.

## Importance
Without a message processing loop, agents may have unprocessed messages in their queues, leading to incomplete execution and potential memory leaks. The loop ensures all agent interactions complete before system shutdown.

## Example
```c
// Basic message processing loop implementation
void process_all_agent_messages(ar_system_t *mut_system) {
    printf("Processing messages...\n");
    int messages_processed = ar_system__process_all_messages_with_instance(mut_system);
    
    // User-friendly output with singular/plural handling
    if (messages_processed > 0) {
        printf("Processed %d message%s\n", messages_processed, 
               messages_processed == 1 ? "" : "s");
    } else {
        printf("No messages to process\n");
    }
}

// The underlying implementation (from ar_system.c)
int ar_system__process_all_messages_with_instance(ar_system_t *mut_system) {
    int count = 0;
    
    while (ar_system__process_next_message_with_instance(mut_system)) {
        count++;
    }
    
    return count;
}
```

## Generalization
Message processing loops are essential in event-driven architectures where:
- Components communicate asynchronously via messages
- Messages may trigger creation of new messages
- Processing order affects system behavior
- Complete drainage of queues is required for clean shutdown

The pattern applies to any system with message queues that need complete processing.

## Implementation
```c
// In executable or main loop
int ar_executable__main(void) {
    // ... initialization code ...
    
    // Create initial agent
    int64_t initial_agent = ar_system__init_with_instance(mut_system, 
                                                         "bootstrap", "1.0.0");
    if (initial_agent <= 0) {
        printf("Error: Failed to create bootstrap agent\n");
        return 1;
    }
    
    // Process all messages until none remain
    printf("Processing messages...\n");
    int messages_processed = ar_system__process_all_messages_with_instance(mut_system);
    printf("Processed %d message%s\n", messages_processed,
           messages_processed == 1 ? "" : "s");
    
    // ... shutdown code ...
}
```

## Related Patterns
- [Message Ownership Flow](message-ownership-flow.md)
- [System Message Flow Architecture](system-message-flow-architecture.md)