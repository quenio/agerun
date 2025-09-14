# Instance vs Global Resource Access Pattern

## Learning
Modules should use instance-specific resources passed during creation rather than accessing global singletons. This pattern emerged when send_instruction_evaluator was incorrectly using the global agency functions instead of the instance-specific ones passed to it.

## Importance
Global resource access creates several problems:
- Breaks in test environments where instances are isolated
- Prevents multiple instances from coexisting
- Makes code harder to test and reason about
- Creates hidden dependencies that aren't visible in the API

## Example
```c
// BAD: Using global function (will fail in tests)
ar_agency__send_to_agent_with_instance(agent_id, message);  // Global singleton access

// GOOD: Using instance passed during creation  
typedef struct {
    ar_agency_t* ref_agency;  // Instance reference stored during creation
} evaluator_t;  // EXAMPLE: Hypothetical type for demonstration

// In the evaluate function:
ar_agency__send_to_agent_with_instance_with_instance(
    evaluator->ref_agency,  // EXAMPLE: Using instance from struct
    agent_id, 
    message
);
```

## Generalization
When designing modules:
1. Accept resource instances as parameters during creation
2. Store references to these instances in the module's struct
3. Use instance-specific APIs (functions ending with `_with_instance`)
4. Never access global singletons from within evaluators or similar modules
5. Pass instances through the entire call chain

## Implementation
To refactor from global to instance-based:
```bash
# Find global function usage
grep -r "ar_agency__send_to_agent_with_instance[^_]" modules/

# Look for _with_instance variants
grep -r "_with_instance" modules/*.h

# Update module creation to accept instance
# Update struct to store instance reference
# Change all calls to use instance variant
```

## Related Patterns
- [Opaque Types Principle](opaque-types-principle.md)
- [Single Responsibility Principle](single-responsibility-principle.md)