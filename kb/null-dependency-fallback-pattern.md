# NULL Dependency Fallback Pattern

## Learning
When instance APIs accept NULL for dependencies, they should consistently fall back to global APIs throughout all operations (init, process, cleanup). This ensures proper resource management in hybrid scenarios where instance APIs use global resources.

## Importance
Without consistent fallback behavior, resources may not be properly initialized or cleaned up. For example, if an agency has no methodology (NULL), the system must use global methodology functions for both loading and cleanup to maintain symmetry.

## Example
```c
// From ar_system module - handling NULL agency and NULL methodology
void ar_system__init(ar_system_t *mut_system, 
                                   const char *ref_method_name, 
                                   const char *ref_version) {
    if (mut_system->ref_agency) {
        // Have custom agency - check if it has methodology
        ar_methodology_t *ref_methodology = ar_agency__get_methodology(mut_system->ref_agency);
        
        if (ref_methodology) {
            // Use instance methodology
            ar_methodology__load_methods(ref_methodology, NULL);
        } else {
            // Agency exists but has NULL methodology - use global
            ar_methodology__load_methods();
        }
        
        // Always use instance agency when available
        ar_agency__load_agents(mut_system->ref_agency, NULL);
    } else {
        // NULL agency - use all global functions
        ar_methodology__load_methods();
        ar_agency__load_agents();
    }
}

void ar_system__shutdown(ar_system_t *mut_system) {
    if (mut_system->ref_agency) {
        ar_methodology_t *ref_methodology = ar_agency__get_methodology(mut_system->ref_agency);
        
        if (ref_methodology) {
            // Save using instance methodology
            ar_methodology__save_methods(ref_methodology, NULL);
        } else {
            // CRITICAL: Must use global save to match global load
            ar_methodology__save_methods();
            ar_methodology__cleanup();  // And cleanup global
        }
        
        ar_agency__save_agents(mut_system->ref_agency, NULL);
        ar_agency__reset(mut_system->ref_agency);
    } else {
        // NULL agency - use all global functions
        ar_methodology__save_methods();
        ar_agency__save_agents();
        ar_agency__reset();
        ar_methodology__cleanup();
    }
}

// Message processing also follows the pattern
bool ar_system__process_next_message(ar_system_t *mut_system) {
    int64_t agent_id;
    ar_data_t *own_message = NULL;
    
    if (mut_system->ref_agency) {
        // Use instance-based agency functions
        agent_id = ar_agency__get_first_agent(mut_system->ref_agency);
        // ... process with instance functions
    } else {
        // Use global agency functions
        agent_id = ar_agency__get_first_agent();
        // ... process with global functions
    }
    
    // Interpreter is always instance-owned
    ar_interpreter__execute_method(mut_system->own_interpreter, agent_id, own_message);
}
```

## Generalization
When designing instance APIs that accept NULL dependencies:
1. **Check NULL at each operation** - Don't assume non-NULL from init
2. **Symmetric operations** - If you load with global, cleanup with global
3. **Document fallback behavior** - Make it clear when global APIs are used
4. **Consistent patterns** - Same NULL check pattern in init/process/cleanup
5. **Test NULL scenarios** - Explicitly test with NULL dependencies

## Implementation
```c
// Pattern for consistent NULL handling
typedef struct {
    dependency_t *ref_dependency;  // EXAMPLE: Generic dependency type - May be NULL
    resource_t *own_resource;      // EXAMPLE: Generic resource type - Always created
} module_t;  // EXAMPLE: Generic module type

void module__operation_with_instance(module_t *mut_module) {  // EXAMPLE: Generic type
    if (mut_module->ref_dependency) {
        // Path 1: Use instance dependency
        dependency__do_something_with_instance(mut_module->ref_dependency);
    } else {
        // Path 2: Use global dependency
        dependency__do_something();  // Global fallback
    }
}

// Pattern for create functions accepting NULL
ar_data_t* module__create(ar_agency_t *ref_dependency) {  // EXAMPLE: Using real types as example
    ar_data_t *own_module = ar_data__create_map();
    if (!own_module) return NULL;
    
    // Store potentially NULL dependency
    if (ref_dependency) {
        ar_data__set_map_data(own_module, "dependency", 
                              ar_data__create_integer((int64_t)ref_dependency));
    }
    
    // Create owned resources regardless
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        ar_data__destroy(own_module);
        return NULL;
    }
    ar_data__set_map_data(own_module, "log", 
                          ar_data__create_integer((int64_t)own_log));
    
    return own_module;
}

// Usage showing both patterns
void example_usage(void) {
    // Pattern 1: With custom dependency
    ar_agency_t *own_agency = ar_agency__create(NULL);
    ar_data_t *own_module1 = module__create(own_agency);  // EXAMPLE: module__create
    module__operation_with_instance(own_module1);  // EXAMPLE: Uses instance
    
    // Pattern 2: With NULL dependency  
    ar_data_t *own_module2 = module__create(NULL);  // EXAMPLE: module__create
    module__operation_with_instance(own_module2);  // EXAMPLE: Uses global
    
    // Cleanup
    ar_data__destroy(own_module1);
    ar_data__destroy(own_module2);
    ar_agency__destroy(own_agency);
}
```

## Related Patterns
- [Global to Instance API Migration](global-to-instance-api-migration.md)
- [Instance API Resource Cleanup Pattern](instance-api-resource-cleanup-pattern.md)
- [Global Instance Wrapper Anti-Pattern](global-instance-wrapper-anti-pattern.md)