# Global to Instance API Migration Pattern

## Learning
Converting modules from global state to instance-based APIs while maintaining backward compatibility requires a specific pattern: create an opaque type, add instance-based functions with `_with_instance` suffix, then delegate existing functions through a global instance wrapper.

## Importance
This pattern enables gradual migration to multi-instance architectures without breaking existing code. It's essential for making modules testable in isolation and supporting future features like multiple runtime instances.

## Example
```c
// Step 1: Add opaque type and structure
typedef struct ar_agency_s ar_agency_t;

struct ar_agency_s {
    bool is_initialized;
    ar_agent_registry_t *own_registry;
    ar_methodology_t *ref_methodology;  // Can be NULL for backward compatibility
};

// Step 2: Keep global state for compatibility
static ar_agency_t *g_default_agency = NULL;

// Step 3: Create helper to get/create global instance
static ar_agency_t* _get_global_instance(void) {
    if (!g_default_agency && g_is_initialized && g_own_registry) {
        g_default_agency = AR__HEAP__MALLOC(sizeof(ar_agency_t), "agency global instance");
        if (g_default_agency) {
            g_default_agency->is_initialized = true;
            g_default_agency->own_registry = g_own_registry;
            g_default_agency->ref_methodology = NULL;
        }
    }
    return g_default_agency;
}

// Step 4: Add instance-based functions
int ar_agency__count_agents(ar_agency_t *ref_agency) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return 0;
    }
    return ar_agent_registry__count(ref_agency->own_registry);
}

// Step 5: Delegate existing functions
int ar_agency__count_agents(void) {
    ar_agency_t *ref_agency = _get_global_instance();
    return ref_agency ? ar_agency__count_agents(ref_agency) : 0;
}
```

## Generalization
1. Create opaque type and internal structure
2. Add create/destroy functions for instances
3. Create instance-based versions of all functions (add `_with_instance` suffix)
4. Maintain global state variables temporarily
5. Create global instance wrapper pattern
6. Delegate all existing functions to instance-based versions
7. Update documentation to show both APIs

## Implementation
This migration preserves backward compatibility while enabling new use cases:
- Existing code continues working unchanged
- New code can create independent instances
- Tests can create isolated instances
- Future refactoring can eliminate global state gradually

### Completing the Migration
After establishing instance-based APIs, the final step is migrating all client code:
1. Update all tests to use instance-based APIs
2. Create migration scripts for systematic updates
3. Fix memory leaks revealed by proper instance management
4. Remove obsolete modules that are no longer needed
5. Document the architectural transformation

See [Migration Script Systematic Updates](migration-script-systematic-updates.md) for automation strategies.

## Related Patterns
- [Module Instantiation Prerequisites](module-instantiation-prerequisites.md)
- [Instance Association Pattern](instance-association-pattern.md)
- [Global Instance Wrapper Anti-Pattern](global-instance-wrapper-anti-pattern.md)
- [Stateless Module Verification](stateless-module-verification.md)
- [API Suffix Cleanup Pattern](api-suffix-cleanup-pattern.md)
- [Global Function Removal Script Pattern](global-function-removal-script-pattern.md)
- [Compilation-Driven Refactoring Pattern](compilation-driven-refactoring-pattern.md)