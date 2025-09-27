# Context Ownership Lifecycle

## Learning

Resource ownership isn't just about who allocates memory, but who has the variables in scope to properly destroy it. When transferring ownership across scope boundaries, destruction must happen where the owning variables are still accessible, not after they've gone out of scope.

## Importance

This pattern prevents:
- Use-after-free when destroying through invalid pointers
- Memory leaks from inaccessible resources
- Crashes from destroying already-freed memory
- Confusion about ownership responsibility

## Example

```c
// WRONG: Destroying after variables out of scope
static void run_first_session(void) {
    ar_data_t *own_echo_context = ar_data__create_map();
    ar_data_t *own_counter_context = ar_data__create_map();
    
    int64_t echo_agent = ar_agency__create_agent(
        mut_agency, "echo", "1.0.0", own_echo_context);
    int64_t counter_agent = ar_agency__create_agent(
        mut_agency, "counter", "1.0.0", own_counter_context);
    
    // ... use agents ...
    
    ar_system__shutdown(mut_system);
    // Variables own_echo_context, own_counter_context out of scope!
}

int main() {
    run_first_session();
    // Can't destroy contexts here - variables don't exist
    ar_data__destroy(own_echo_context);  // CRASH: undefined variable
}

// CORRECT: Destroying where variables are accessible
int main() {
    // Create contexts in main scope
    ar_data_t *own_echo_context = ar_data__create_map();
    ar_data_t *own_counter_context = ar_data__create_map();
    
    // Initialize and run
    ar_system__init(mut_system, "main", "1.0.0");
    
    // Create agents with contexts
    int64_t echo_agent = ar_agency__create_agent(
        mut_agency, "echo", "1.0.0", own_echo_context);
    
    // ... run session ...
    
    // Shutdown FIRST (while agents still exist)
    ar_system__shutdown(mut_system);
    
    // NOW destroy contexts (variables still in scope)
    ar_data__destroy(own_echo_context);
    ar_data__destroy(own_counter_context);
    
    // Finally destroy system
    ar_system__destroy(mut_system);
}
```

## Generalization

For proper lifecycle management:
1. Create resources in the scope where they'll be destroyed
2. Pass references down, return ownership up
3. Destroy in reverse order of creation
4. Ensure variables are accessible at destruction time
5. Document ownership transfer points clearly

## Implementation

```c
// Pattern for ownership lifecycle
typedef struct {
    ar_data_t *own_resource;  // Owned by this scope
    ar_data_t *ref_borrowed;  // Borrowed from parent scope
} scoped_resources_t;  // EXAMPLE: Pattern for documentation

void process_with_proper_lifecycle(void) {
    // 1. Create in destruction scope
    ar_data_t *own_context = ar_data__create_map();
    
    // 2. Use resource (may pass references down)
    int64_t agent = ar_agency__create_agent(
        mut_agency, "worker", "1.0.0", own_context);
    
    // 3. Process (resource may be borrowed internally)
    ar_system__process_next_message(mut_system);
    
    // 4. Cleanup in reverse order
    ar_agency__destroy_agent(mut_agency, agent);
    
    // 5. Destroy where created (variables in scope)
    ar_data__destroy(own_context);
}
```

## Related Patterns
- [Temporary Resource Ownership Pattern](temporary-resource-ownership-pattern.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Test Memory Leak Ownership Analysis](test-memory-leak-ownership-analysis.md)