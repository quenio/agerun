# Instance API Resource Cleanup Pattern

## Learning
Instance-level functions must only clean up resources they own, never borrowed references. When a module holds a borrowed reference (ref_ prefix), it should not call destroy or cleanup functions on that resource.

## Importance
Violating this principle leads to double-frees, use-after-free bugs, and ownership confusion. The borrower of a reference has no knowledge of when the owner might destroy the resource, so attempting cleanup violates the ownership contract.

## Example
```c
// From ar_system module refactoring - WRONG approach
struct ar_system_s {
    ar_agency_t *ref_agency;         // Borrowed reference
    ar_interpreter_t *own_interpreter; // Owned by system
    ar_log_t *own_log;               // Owned by system
};

void ar_system__shutdown_with_instance(ar_system_t *mut_system) {
    // WRONG: Trying to get methodology from agency
    ar_methodology_t *ref_methodology = ar_agency__get_methodology(mut_system->ref_agency);
    if (ref_methodology) {
        // WRONG: System doesn't own methodology!
        ar_methodology__cleanup_with_instance(ref_methodology);  // EXAMPLE: Don't do this!
    }
    
    // CORRECT: Only clean up owned resources
    if (mut_system->own_interpreter) {
        ar_interpreter__destroy(mut_system->own_interpreter);
    }
    if (mut_system->own_log) {
        ar_log__destroy(mut_system->own_log);
    }
    // Note: ref_agency is NOT destroyed - not owned by system
}

// From ar_agency module - shows ownership model
struct ar_agency_s {
    ar_agent_registry_t *own_registry;  // Owned by agency
    ar_methodology_t *ref_methodology;  // Borrowed reference - NOT owned
};

void ar_agency__destroy(ar_agency_t *own_agency) {
    // CORRECT: Destroy owned registry
    if (own_agency->own_registry) {
        ar_agent_registry__destroy(own_agency->own_registry);
    }
    // CORRECT: Do NOT destroy ref_methodology - not owned
    
    AR__HEAP__FREE(own_agency);
}
```

## Generalization
When implementing cleanup/destroy functions:
1. **Check prefixes** - Only destroy resources with `own_` prefix
2. **Ignore borrowed** - Never cleanup `ref_` or `mut_` prefixed resources
3. **Document ownership** - Make ownership clear in comments
4. **Verify at boundaries** - Double-check ownership when crossing module boundaries
5. **Test destruction order** - Ensure resources are destroyed by their owners

## Implementation
```c
// Pattern for safe cleanup
void module__destroy(module_t *own_module) {  // EXAMPLE: Generic module type
    if (!own_module) return;
    
    // Step 1: Destroy owned sub-resources
    if (own_module->own_resource1) {
        resource1__destroy(own_module->own_resource1);
    }
    if (own_module->own_resource2) {
        resource2__destroy(own_module->own_resource2);
    }
    
    // Step 2: Clear borrowed references (optional, for safety)
    own_module->ref_dependency = NULL;
    own_module->mut_shared = NULL;
    
    // Step 3: Free the module itself
    AR__HEAP__FREE(own_module);
}

// Pattern for conditional cleanup based on ownership
void system__cleanup_resources(system_t *mut_system) {  // EXAMPLE: Generic system type
    // If using instance methodology, let owner handle cleanup
    if (mut_system->ref_methodology) {
        // Just save, don't cleanup - not owned
        ar_methodology__save_methods_with_instance(mut_system->ref_methodology, NULL);
    } else {
        // Using global methodology - we're responsible
        ar_methodology__save_methods_with_instance();
        ar_methodology__cleanup_with_instance();  // Global cleanup is OK here
    }
}
```

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [API Ownership on Failure](api-ownership-on-failure.md)
- [Temporary Resource Ownership Pattern](temporary-resource-ownership-pattern.md)
- [Global Instance Wrapper Anti-Pattern](global-instance-wrapper-anti-pattern.md)