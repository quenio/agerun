# Instance Association Pattern

## Learning
When making modules instantiable, associated modules should be passed as instance fields during construction rather than accessed globally. This creates clear ownership relationships and enables multiple independent instances.

## Importance
Proper instance association ensures modules can be tested in isolation, supports multiple runtime instances, and eliminates hidden global dependencies that complicate testing and deployment.

## Example
```c
// BAD: Hidden global dependencies
typedef struct ar_agency_s {  // EXAMPLE: Future type after refactoring
    ar_agent_registry_t* own_registry;
    // Accesses global methodology instance internally!
} ar_agency_t;  // EXAMPLE: Future type after refactoring

ar_method_t* _get_method_for_agent(const char* name) {
    // Hidden dependency on global methodology
    return ar_methodology__get_method_with_instance(name, NULL);  
}

// GOOD: Explicit instance association
typedef struct ar_agency_s {  // EXAMPLE: Future type after refactoring
    ar_methodology_t* own_methodology;  // Explicit ownership
    ar_agent_registry_t* own_registry;
} ar_agency_t;  // EXAMPLE: Future type after refactoring

ar_agency_t* ar_agency__create(ar_methodology_t* methodology) {  // EXAMPLE: Future function
    ar_agency_t* agency = AR__HEAP__MALLOC(sizeof(ar_agency_t));  // EXAMPLE: Using future type
    agency->own_methodology = methodology;  // Clear association
    agency->own_registry = ar_agent_registry__create();
    return agency;
}

// Now agency methods use associated instance:
ar_method_t* _get_method_for_agent(ar_agency_t* agency, const char* name) {  // EXAMPLE: Using future type
    return ar_methodology__get_method_with_instance(
        agency->own_methodology, name, NULL
    );
}
```

## Generalization
**Instance Association Principles**:
1. **Explicit dependencies**: Pass all dependencies to constructor
2. **Clear ownership**: Document whether instance owns or borrows dependency
3. **No global access**: Never access global instances from instance methods
4. **Hierarchical relationships**: Follow natural module hierarchy
5. **Backward compatibility**: Maintain global functions that delegate to default instance

**Ownership Patterns**:
- `own_`: Instance owns and will destroy the dependency
- `ref_`: Instance borrows the dependency (someone else owns it)
- Constructor decides ownership based on use case

## Implementation
1. **Add instance fields** for all dependencies
2. **Update constructor** to accept dependency instances
3. **Convert methods** to use instance fields instead of globals
4. **Document ownership** in header comments
5. **Update tests** to create proper instance hierarchies

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Dependency Injection Anti Pattern](dependency-injection-anti-pattern.md)
- [Composition Over Inheritance Principle](composition-over-inheritance-principle.md)