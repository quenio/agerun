# Header Inclusion Over Forward Declaration

## Learning
AgeRun codebase requires including actual module headers instead of using forward declarations for struct types. When a function signature uses `ar_delegate_t*` or similar types, must include the defining header (e.g., `ar_delegate.h`), not forward declare.

## Importance
Forward declarations hide dependencies and make the compilation dependency graph unclear. Including actual headers makes dependencies explicit, ensures type definitions are available, and prevents subtle compilation issues. This aligns with AgeRun's principle of explicit, transparent code.

## Example
```c
// WRONG: Forward declarations in header
// File: modules/ar_system.h
typedef struct ar_delegate_s ar_delegate_t;  // Forward declaration
typedef struct ar_delegate_registry_s ar_delegate_registry_t;  // Forward declaration

ar_delegate_registry_t* ar_system__get_delegate_registry(const ar_system_t *ref_system);
bool ar_system__register_delegate(ar_system_t *mut_system,
                                int64_t proxy_id,
                                ar_delegate_t *own_proxy);

// PROBLEMS:
// 1. Dependency on ar_delegate module is hidden
// 2. Type definitions not available for inline functions
// 3. Compilation dependency graph unclear
// 4. Users of ar_system.h might need to include ar_delegate.h anyway

// CORRECT: Include actual headers
// File: modules/ar_system.h
#include "ar_delegate.h"
#include "ar_delegate_registry.h"

ar_delegate_registry_t* ar_system__get_delegate_registry(const ar_system_t *ref_system);
bool ar_system__register_delegate(ar_system_t *mut_system,
                                int64_t proxy_id,
                                ar_delegate_t *own_proxy);

// BENEFITS:
// 1. Dependencies are explicit and visible
// 2. Type definitions available throughout
// 3. Clear compilation order
// 4. Users get all necessary types automatically
```

## Generalization
**Header Inclusion Strategy:**

1. **In Public Headers** (modules/ar_system.h):
   ```c
   // Include headers for all types used in function signatures
   #include "ar_data.h"      // For ar_data_t* parameters
   #include "ar_log.h"       // For ar_log_t* parameters
   #include "ar_agent.h"     // For ar_agent_t* return types

   // Declare functions using those types
   ar_data_t* ar_system__process(ar_system_t *mut_system,  // EXAMPLE: Hypothetical function
                                  ar_agent_t *ref_agent,
                                  ar_log_t *ref_log);
   ```

2. **In Implementation Files** (modules/ar_system.c):
   ```c
   #include "ar_system.h"    // Own header first
   #include "ar_heap.h"      // For memory allocation
   #include "ar_assert.h"    // For assertions
   // Other needed headers
   ```

3. **In Test Files** (modules/ar_system_tests.c):
   ```c
   #include "ar_system.h"    // Module under test
   #include "ar_assert.h"    // For AR_ASSERT
   #include "ar_data.h"      // For test data
   #include "ar_log.h"       // For test fixtures
   // All types used in tests
   ```

**When Forward Declarations Might Seem Useful:**
- Reducing compilation dependencies
- Breaking circular dependencies
- Speeding up compilation

**Why AgeRun Doesn't Use Them:**
- Explicit > implicit (clarity principle)
- Type information should be complete
- Circular dependencies should be refactored, not hidden
- Compilation speed is secondary to code clarity

## Implementation
```bash
# Finding forward declarations that should be includes
grep -n "typedef struct.*_s.*_t;" modules/*.h

# Checking what types are used in function signatures
grep -A5 "^[a-z_]*_t\*" modules/ar_system.h

# Finding which header defines a type
grep "typedef struct ar_delegate_s" modules/*.h
# Result: modules/ar_delegate.h

# Replace forward declaration with include
# Old:
typedef struct ar_delegate_s ar_delegate_t;

# New:
#include "ar_delegate.h"
```

## Real Example from TDD Cycle 4.5
During integration of ar_delegate_registry into ar_system:

**Initial mistake:**
```c
// modules/ar_system.h
/* Forward declarations */
typedef struct ar_delegate_s ar_delegate_t;
typedef struct ar_delegate_registry_s ar_delegate_registry_t;
```

**User correction:**
> "You also should know by now that forward declarations are not allowed; we should include the module that defines the struct."

**Correct approach:**
```c
// modules/ar_system.h
#include "ar_delegate.h"
#include "ar_delegate_registry.h"

// Now functions can use these types with full definitions
ar_delegate_registry_t* ar_system__get_delegate_registry(const ar_system_t *ref_system);
bool ar_system__register_delegate(ar_system_t *mut_system,
                                int64_t proxy_id,
                                ar_delegate_t *own_proxy);
```

## Exception: Opaque Types
AgeRun uses opaque type pattern where implementation struct is hidden:

```c
// In modules/ar_system.h - PUBLIC HEADER
typedef struct ar_system_s ar_system_t;  // Opaque declaration for OWN type
#include "ar_delegate.h"                     // Full include for DEPENDENCY types

// In modules/ar_system.c - IMPLEMENTATION
struct ar_system_s {  // Full definition hidden in .c file
    ar_delegate_registry_t *own_registry;
    // ... other fields
};
```

**Rule:** Only forward declare your OWN type in your OWN header. Always include headers for DEPENDENCY types.

## Related Patterns
- [Opaque Types Pattern](opaque-types-pattern.md) - When to hide implementation
- [No Circular Dependencies Principle](no-circular-dependencies-principle.md) - Avoiding dependency cycles
- [Module Development Patterns](module-development-patterns.md) - Module structure guidelines
- [Separation of Concerns Principle](separation-of-concerns-principle.md) - Module boundaries
