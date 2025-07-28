# Internal vs External Module Pattern

## Learning
Modules in AgeRun follow a clear distinction between internal implementation modules (like registries) and external interface modules (like stores). Understanding this distinction is crucial for proper architectural decisions.

## Importance
Misunderstanding module roles can lead to exposing internal implementation details or creating incorrect dependencies. Recognizing whether a module is internal or external guides proper usage and interface design.

## Example
In the agency module architecture:
```c
// INTERNAL modules (implementation details):
// ar_agent_registry - tracks agents internally
ar_agent_registry_t* registry;  // Not exposed in public API
ar_agent_registry__register(registry, agent_id, agent);

// EXTERNAL modules (public persistence):
// ar_agent_store - saves/loads agents  
ar_agent_store__save_all_agents(filename);  // Public API for persistence

// FACADE module (public interface):
// ar_agency - coordinates internal modules
ar_agency__create_agent(name, method, version);  // Public API
```

Similarly for methodology:
```c
// INTERNAL: ar_method_registry (storage)
// INTERNAL: ar_method_resolver (resolution logic)  
// PUBLIC: ar_methodology (facade coordinating both)

// Users only interact with:
ar_methodology__get_method("name", "1.2");  // Public API
// Not with:
ar_method_registry__get_latest_version(...);  // Internal
```

## Generalization
1. **Internal modules** are implementation details:
   - Named with "_registry" for tracking/indexing
   - Named with "_resolver" for logic/algorithms
   - Not exposed in public headers
   - Can change without breaking client code

2. **External modules** are public interfaces:
   - Named with "_store" for persistence
   - May be exposed for configuration
   - Have stable APIs

3. **Facade modules** coordinate internal modules:
   - Provide the public API
   - Hide internal complexity
   - Delegate to appropriate internal modules

## Implementation
```c
// When designing module relationships:

// 1. Identify the module type
if (module_tracks_objects_internally) {
    // It's a registry (internal)
    module_name = "ar_thing_registry";  
    visibility = "internal to facade";
}

if (module_handles_persistence) {
    // It's a store (external interface)
    module_name = "ar_thing_store";
    visibility = "may be public";
}

// 2. Design the facade
// Facade module coordinates internal modules
// Example: ar_methodology coordinates registry + resolver
ar_methodology_t* methodology = ar_methodology__create(NULL);
// Internally uses: registry, resolver
// Exposes: simple public API
```

## Related Patterns
- [Separation of Concerns Principle](separation-of-concerns-principle.md)
- [Information Hiding Principle](information-hiding-principle.md)
- [Architectural Patterns Hierarchy](architectural-patterns-hierarchy.md)