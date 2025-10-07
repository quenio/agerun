# AR_ASSERT for Descriptive Test Failures

## Learning
The ar_assert module provides `AR_ASSERT(condition, message)` macro that includes custom error messages in assertion failures, making test debugging much easier than plain `assert()`.

## Importance
When tests fail, descriptive error messages immediately tell you what was expected vs what happened, eliminating the need to look up line numbers and decipher cryptic assertion failures. This is especially valuable in CI/CD environments where debugging access is limited.

## Example
```c
#include "ar_assert.h"
#include <stdbool.h>
#include <sys/stat.h>

static void test_no_auto_saving_on_shutdown(void) {
    // Given a system with methodology and agents
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");
    
    ar_system__init(mut_system, NULL, NULL);
    
    // When the system is shut down
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    
    // Then no files should be saved
    struct stat st;
    bool methodology_exists = (stat("methodology.agerun", &st) == 0);
    bool agency_exists = (stat("agerun.agency", &st) == 0);
    
    AR_ASSERT(!methodology_exists, "methodology.agerun should NOT have been saved on shutdown");
    AR_ASSERT(!agency_exists, "agerun.agency should NOT have been saved on shutdown");
}
```

When this fails, you get:
```
Assertion failed: ((!methodology_exists) && "methodology.agerun should NOT have been saved on shutdown"), function test_no_auto_saving_on_shutdown, file ar_system_tests.c, line 289.
```

## Generalization
Replace all test assertions with AR_ASSERT to provide context:
- State what should happen: `AR_ASSERT(result != NULL, "Function should return valid pointer")`
- Explain expectations: `AR_ASSERT(count == 5, "Should have exactly 5 elements after insertion")`
- Describe invariants: `AR_ASSERT(own_ptr == NULL, "Pointer should be NULL after ownership transfer")`

## Implementation
```c
// 1. Include the header
#include "ar_assert.h"

// 2. Replace plain assertions
// Before:
assert(value > 0);

// After:
AR_ASSERT(value > 0, "Value should be positive after initialization");

// 3. For complex conditions, extract to boolean for clarity
bool valid_signal = (sig == SIGTERM || sig == SIGALRM);
AR_ASSERT(valid_signal, "Process should terminate by SIGTERM or SIGALRM");

// 4. The macro is DEBUG-only, becomes no-op in release builds
#ifdef DEBUG
#define AR_ASSERT(cond, msg) assert((cond) && msg)
#else
#define AR_ASSERT(cond, msg) ((void)0)
#endif
```

## Related Patterns
- [BDD Test Structure](bdd-test-structure.md)
- [Standards Over Expediency Principle](standards-over-expediency-principle.md)
- [Test Error Marking Strategy](test-error-marking-strategy.md)
- [Evidence-based Debugging](evidence-based-debugging.md)
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)
- [Test Complexity as Code Smell](test-complexity-as-code-smell.md)