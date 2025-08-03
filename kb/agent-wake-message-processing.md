# Agent Wake Message Processing

## Learning
All agents automatically send themselves a `__wake__` message upon creation. Tests must process this message to prevent memory leaks, as unprocessed messages remain in the agent's queue and are never freed.

## Importance
This pattern is critical for preventing memory leaks in tests. During the session, 8+ test files had identical memory leaks (2 allocations, ~41-45 bytes each) caused by unprocessed wake messages. Understanding this pattern prevents systematic memory leaks across the test suite.

## Example
```c
// WRONG: Creates memory leak
int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, "test_method", "1.0.0", NULL);
// Wake message sits in queue unprocessed

// CORRECT: Process wake message immediately
int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, "test_method", "1.0.0", NULL);
ar_system__process_next_message_with_instance(own_system);  // Processes and frees wake message

// In test cleanup, ensure all messages are processed
while (ar_system__process_next_message_with_instance(own_system)) {
    // Process any remaining messages
}

// Real example from ar_agency_tests.c fix:
void test_agency__create_agent() {
    ar_agent_t *ref_agent = ar_agency__create_agent_with_instance(mut_agency, "test_agent", "1.0.0", NULL);
    assert(ref_agent != NULL);
    
    // Process the wake message to prevent memory leak
    ar_system__process_next_message_with_instance(own_system);
    
    ar_agency__destroy_agent(1);
    ar_methodology__cleanup();
}
```

## Generalization
Whenever creating agents in tests, immediately process their wake messages. This applies to:
- Direct agent creation via `ar_agency__create_agent_with_instance(mut_agency, )`
- Agent creation through fixtures
- Any code path that results in agent instantiation
- Test setup that creates multiple agents

## Implementation
1. After any `ar_agency__create_agent_with_instance(mut_agency, )` call, add `ar_system__process_next_message_with_instance(own_system)`
2. In test cleanup, process all remaining messages with a while loop
3. When creating multiple agents, call `process_next_message()` after each creation
4. For fixtures that create agents, ensure the fixture handles wake message processing

```c
// Pattern for multiple agents
for (int i = 0; i < num_agents; i++) {
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, "method", "1.0.0", NULL);
    ar_system__process_next_message_with_instance(own_system);  // Process each wake message
}
```


**Note**: Examples assume `own_system`, `mut_agency`, and other instance variables are available. In practice, these would be created via fixtures or passed as parameters.
## Related Patterns
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
- [Test Fixture API Adaptation](test-fixture-api-adaptation.md)
- [Ownership Transfer in Message Passing](ownership-drop-message-passing.md)