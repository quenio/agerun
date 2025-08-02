# Module Instantiation Prerequisites

## Learning
When planning to make a module instantiable, you must first analyze its dependencies and ensure prerequisite modules are instantiable. Dependencies should be instantiated bottom-up following the module hierarchy.

## Importance
Attempting to make a module instantiable without first handling its dependencies leads to architectural inconsistencies, global state conflicts, and complex workarounds. Proper prerequisite analysis ensures clean instance relationships.

## Example
```c
// Discovered during system module refactoring:
// System depends on Agency depends on Methodology

// WRONG: Trying to make system instantiable first
typedef struct ar_system_s {  // EXAMPLE: Future type after refactoring
    ar_methodology_t* methodology;  // But agency still uses global methodology!
    // This creates inconsistency...
} ar_system_t;  // EXAMPLE: Future type after refactoring

// RIGHT: Follow dependency order
// 1. Methodology is already instantiable ✓
// 2. Make agency instantiable with methodology field:
typedef struct ar_agency_s {  // EXAMPLE: Future type after refactoring
    ar_methodology_t* own_methodology;  // Agency owns its methodology
    ar_agent_registry_t* own_registry;
    bool is_initialized;
} ar_agency_t;  // EXAMPLE: Future type after refactoring

// 3. Then make system instantiable:
typedef struct ar_system_s {  // EXAMPLE: Future type after refactoring
    ar_agency_t* own_agency;  // System owns its agency (which owns methodology)  // EXAMPLE: Using future type
    ar_log_t* own_log;
    // Clean hierarchy: System → Agency → Methodology
} ar_system_t;  // EXAMPLE: Future type after refactoring
```

## Generalization
**Module Instantiation Order**:
1. Analyze the full dependency graph
2. Start with leaf modules (no dependencies)
3. Work up the hierarchy level by level
4. Each module should accept its dependencies as constructor parameters
5. Maintain backward compatibility with global instance pattern

**Dependency Analysis Commands**:
```bash
# Find what a module depends on
grep "#include.*ar_" modules/ar_module.h modules/ar_module.c

# Find what depends on a module
grep -l "ar_module.h" modules/*.c modules/*.h
```

## Implementation
1. **Map dependencies**: Create a dependency graph of modules
2. **Identify prerequisites**: Find modules that must be instantiable first
3. **Plan phases**: Order refactoring phases by dependency hierarchy
4. **Update interfaces**: Each create function accepts dependency instances
5. **Test incrementally**: Verify each phase maintains functionality

## Related Patterns
- [Module Delegation Error Propagation](module-delegation-error-propagation.md)
- [No Circular Dependencies Principle](no-circular-dependencies-principle.md)
- [Internal vs External Module Pattern](internal-vs-external-module-pattern.md)
- [Global to Instance API Migration](global-to-instance-api-migration.md)