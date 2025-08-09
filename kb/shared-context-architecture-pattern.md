# Shared Context Architecture Pattern

## Learning
System instances should maintain ONE shared context for all child components (agents) they create, not individual contexts per component. This prevents memory leaks and ensures consistent state across all components within a system boundary.

## Importance
- Prevents memory leaks from duplicated resources
- Ensures consistent shared state across all agents
- Simplifies lifecycle management (one resource to create/destroy)
- Reflects proper system-level resource ownership
- Reduces memory overhead in multi-agent systems

## Example
```c
// WRONG: Creating individual context per agent (causes memory leaks)
struct ar_system_s {
    ar_agency_t *own_agency;
    ar_interpreter_t *own_interpreter;
    // No shared context
};

int64_t ar_system__init_with_instance(ar_system_t *mut_system, const char *method, const char *version) {
    // BAD: Creating new context for each agent
    ar_data_t *own_context = ar_data__create_map();  // Memory leak!
    int64_t agent = ar_agency__create_agent_with_instance(mut_system->own_agency, 
                                                         method, version, own_context);
    // Context ownership unclear, likely leaked
}

// RIGHT: System maintains shared context for all agents
struct ar_system_s {
    ar_agency_t *own_agency;
    ar_interpreter_t *own_interpreter;
    ar_data_t *own_context;  // Shared context for all agents
};

ar_system_t* ar_system__create(void) {
    ar_system_t *own_system = AR__HEAP__MALLOC(sizeof(ar_system_t));
    
    // Create shared context once
    own_system->own_context = ar_data__create_map();
    if (!own_system->own_context) {
        AR__HEAP__FREE(own_system);
        return NULL;
    }
    // ... create other resources
}

int64_t ar_system__init_with_instance(ar_system_t *mut_system, const char *method, const char *version) {
    // Use shared context for agent creation
    int64_t agent = ar_agency__create_agent_with_instance(mut_system->own_agency, 
                                                         method, version, 
                                                         mut_system->own_context);
    // Context properly shared, no leak
}

void ar_system__destroy(ar_system_t *own_system) {
    // Destroy shared context once
    if (own_system->own_context) {
        ar_data__destroy(own_system->own_context);
    }
}
```

## Generalization
When designing system-level components:
- Identify resources that should be shared across child components
- Create shared resources in the parent's constructor
- Pass references (not ownership) to child components
- Destroy shared resources in the parent's destructor
- Use ownership prefixes to clarify: parent owns (own_), children reference (ref_)

## Implementation
```c
// Pattern for any system with shared resources:
typedef struct system_s {
    child_manager_t *own_child_manager;  // EXAMPLE: Hypothetical type
    shared_resource_t *own_shared_resource;  // EXAMPLE: Hypothetical type - System owns
} system_t;  // EXAMPLE: Hypothetical struct for teaching

system_t* system__create(void) {  // EXAMPLE: Hypothetical function
    system_t *own_system = AR__HEAP__MALLOC(sizeof(system_t));  // EXAMPLE: Using hypothetical type
    own_system->own_shared_resource = shared_resource__create();  // EXAMPLE: Hypothetical function
    // Pass reference when creating child manager
    own_system->own_child_manager = child_manager__create(own_system->own_shared_resource);  // EXAMPLE: Hypothetical function
    return own_system;
}

void system__add_child(system_t *mut_system) {  // EXAMPLE: Hypothetical function
    // Children receive reference, not ownership
    child_manager__add(mut_system->own_child_manager,  // EXAMPLE: Hypothetical function
                       mut_system->own_shared_resource);  // ref passed
}
```

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Temporary Resource Ownership Pattern](temporary-resource-ownership-pattern.md)
- [Context Ownership Lifecycle](context-ownership-lifecycle.md)