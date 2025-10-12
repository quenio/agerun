# ID-Based Message Routing Pattern

## Learning
Using the mathematical properties of identifier space enables clean multiplexing of different subsystems through a single interface. By assigning ID ranges with different signs (>= 0 for one subsystem, < 0 for another), routing becomes a simple comparison without complex lookup tables or configuration.

## Importance
This pattern provides a simple, efficient way to coordinate multiple peer subsystems through a single facade interface. It eliminates the need for type tags, configuration files, or registry lookups while maintaining clear subsystem boundaries and enabling independent evolution of each subsystem.

## Example
```c
// Real implementation from ar_system coordinating agency and delegation
// File: modules/ar_system.c

typedef struct ar_system_s {
    ar_agency_t *own_agency;          // Handles IDs >= 0 (agents)
    ar_delegation_t *own_delegation;  // Handles IDs < 0 (delegates)
    ar_log_t *own_log;
} ar_system_s;

// Message routing based on ID sign (conceptual example)
bool ar_system__send_message(ar_system_t *mut_system,  // EXAMPLE: Hypothetical function for demonstration
                             int64_t target_id,
                             const ar_data_t *ref_message) {  // EXAMPLE: Function signature
    // Route based on ID sign - simple, efficient
    if (target_id >= 0) {
        // Positive IDs route to agency (agents)
        ar_data_t *mut_memory = ar_agency__get_agent_mutable_memory(
            mut_system->own_agency, target_id);  // Real function
        if (!mut_memory) {
            ar_log_t *ref_log = ar_system__get_log(mut_system);  // Real function
            ar_log__error(ref_log,
                         "Agent not found: %lld", target_id);
            return false;
        }
        return ar_agent__send((ar_agent_t*)mut_memory, (ar_data_t*)ref_message);  // EXAMPLE: Simplified for demonstration
    } else {
        // Negative IDs route to delegation (delegates)
        ar_delegate_registry_t *ref_registry = ar_system__get_delegate_registry(mut_system);  // Real function
        ar_delegate_t *ref_delegate = ar_delegate_registry__get(ref_registry, target_id);  // Real function
        if (!ref_delegate) {
            ar_log_t *ref_log = ar_system__get_log(mut_system);  // Real function
            ar_log__error(ref_log,
                         "Delegate not found: %lld", target_id);
            return false;
        }
        return ar_delegate__handle_message(ref_delegate, (ar_data_t*)ref_message, 0);  // Real function
    }
}

// Example usage creating different entity types
void example_routing() {  // EXAMPLE: Demonstration function
    ar_system_t *mut_system = ar_system__create();  // Real function

    // Create agents with positive IDs (0, 1, 2, ...) using real functions
    int64_t agent_id = ar_system__init(mut_system, "example_method", "1.0");  // Real function - returns ID >= 0

    // Create delegates with negative IDs (-1, -2, -3, ...)
    ar_log_t *ref_log = ar_system__get_log(mut_system);  // Real function
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "external_service");  // Real function
    ar_system__register_delegate(mut_system, -1, own_delegate);  // Real function

    // Messages can route based on ID (conceptual - actual routing via ar_system__process_next_message)
    ar_data_t *own_msg1 = ar_data__create_string("hello agent");  // Real function
    ar_data_t *own_msg2 = ar_data__create_string("hello delegate");  // Real function

    // Real system uses ar_system__process_next_message() for message processing
    // The ID sign determines routing: >= 0 to agency, < 0 to delegation

    // Cleanup
    ar_data__destroy(own_msg1);
    ar_data__destroy(own_msg2);
    ar_system__destroy(mut_system);  // Real function
}
```

## Generalization
**ID Space Partitioning Strategies:**

1. **Sign-Based** (AgeRun approach):
   - Positive IDs (>= 0): One subsystem
   - Negative IDs (< 0): Another subsystem
   - Allows infinite IDs in each space
   - Simple comparison for routing

2. **Range-Based**:
   - IDs 0-999: Subsystem A
   - IDs 1000-1999: Subsystem B
   - IDs 2000+: Subsystem C
   - Requires range management

3. **Bit-Based**:
   - High bit 0: Type A
   - High bit 1: Type B
   - Remaining bits: ID within type
   - Fast bitwise operations

**When to Use This Pattern:**
- Coordinating multiple peer subsystems
- Need simple, efficient routing without lookups
- Subsystems have similar interfaces
- Want to avoid type tags or configuration
- Need clear namespace separation

## Implementation
```bash
#!/bin/bash
# Verification checklist for ID-based routing

echo "=== ID-Based Routing Verification ==="
echo ""

# 1. Check routing logic uses simple comparison
echo "1. Routing logic:"
grep -A 5 "if.*target_id.*>= 0\|if.*id >= 0" modules/ar_system.c

# 2. Verify both positive and negative IDs handled
echo ""
echo "2. Both ID spaces handled:"
grep -c "if.*>= 0" modules/ar_system.c  # Should be at least 1
grep -c "else.*<" modules/ar_system.c   # Should match

# 3. Check ID assignment in registration functions
echo ""
echo "3. Registration preserves ID sign:"
grep -A 3 "register_agent\|register_delegate" modules/ar_system.c

# 4. Verify tests cover both ID spaces
echo ""
echo "4. Test coverage:"
grep "register.*0\|register.*-1" modules/ar_system_tests.c
```

## Benefits
1. **Simplicity**: Single comparison for routing
2. **Efficiency**: No hash lookups or linear searches
3. **Clarity**: ID sign immediately indicates subsystem
4. **Scalability**: Infinite IDs in each space (up to int64_t limits)
5. **Type Safety**: Can't accidentally mix agent and delegate IDs
6. **Extensibility**: Easy to add more subsystems with different ranges

## Architectural Context
This pattern emerges naturally when using the Facade pattern to coordinate peer subsystems:

```
System (Facade)
├── Agency (IDs >= 0)
│   └── Agent Registry
└── Delegation (IDs < 0)
    └── Delegate Registry
```

Each subsystem owns its registry and manages its ID space independently. The facade simply routes based on ID sign.

## Related Patterns
- [Facade Pattern Coordination](facade-pattern-coordination.md)
- [Symmetric Peer Architecture Pattern](architectural-patterns-hierarchy.md)
- [Separation of Concerns Principle](separation-of-concerns-principle.md)
- [Single Responsibility Principle](single-responsibility-principle.md)
