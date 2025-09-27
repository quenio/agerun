# Temporary Resource Ownership Pattern

## Learning

When creating temporary agents with contexts in AgeRun, the creator must handle context cleanup because agents store contexts as references without taking ownership. This creates a gap where resources can be orphaned if not explicitly managed.

## Importance

Prevents memory leaks in temporary resource scenarios where the standard ownership model (agent owns everything) doesn't apply. Critical for fixture-based testing and temporary agent creation patterns.

## Example

**Problem**: Temporary agent contexts leak because agents don't own them

```c
// In ar_interpreter_fixture__execute_with_message()
ar_data_t *own_default_context = ar_data__create_map();  // Create context
int64_t temp_agent_id = ar_agency__create_agent(method_name, "1.0.0", own_default_context);
// Agent stores context as ref_context - doesn't take ownership

// Later: ar_agency__destroy_agent(temp_agent_id)
// Agent destructor: "We don't own the context, just clear the reference"
// Result: Context is orphaned and leaks
```

**Solution**: Creator handles cleanup in destroy function

```c
void ar_interpreter_fixture__destroy_temp_agent(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t temp_agent_id) {
    
    // Get context before destroying agent
    const ar_data_t *ref_context = ar_agency__get_agent_context(temp_agent_id);
    
    // Destroy agent (handles its own resources)
    ar_agency__destroy_agent(temp_agent_id);
    
    // Process remaining messages
    while (system_process_next_message_example()) {
        // Keep processing
    }
    
    // Destroy context we created
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // We created it, we destroy it
        #pragma GCC diagnostic pop
    }
}
```

## Generalization

**Pattern**: When creating temporary resources for agents:

1. **Creation**: Create resource and pass to agent
2. **Storage**: Agent stores as reference (doesn't own)
3. **Retrieval**: Get resource reference before agent destruction
4. **Cleanup**: Explicitly destroy resource after agent destruction

**Applies to**:
- Temporary agent contexts in fixtures
- External resources passed to agents
- Any resource where agent lifetime < resource lifetime needed

## Implementation

**Detection**: Look for memory leaks where:
- Leak addresses match resource creation addresses
- Agent destruction debug shows proper cleanup
- Leaked resources are maps/lists passed to agents

**Debug approach**:
```c
// Add debugging to trace resource lifecycle
fprintf(stderr, "DEBUG: Created context at %p\n", (void*)context);
// ... agent operations ...
fprintf(stderr, "DEBUG: Retrieved context at %p\n", (void*)retrieved_context);
// Verify addresses match between creation and retrieval
```

**Memory reports will show**:
- Agent's own resources (message queue, memory) properly cleaned
- External resources (contexts) as orphaned leaks
- Addresses match between debug output and leak report

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md) - Use own_ for created resources
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md) - Systematic leak debugging
- [Evidence-Based Debugging](evidence-based-debugging.md) - Verify hypotheses with concrete evidence