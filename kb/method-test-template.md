# Method Test Template

## Learning
Standard template for writing method tests that ensures proper directory setup, clean state, and system initialization.

## Importance
Consistent test structure prevents common errors like wrong working directory, state pollution between tests, and memory leaks from unprocessed messages.

## Example
```c
#include <unistd.h>
#include "ar_system.h"
#include "ar_methodology.h"
#include "ar_agency.h"
// ... other includes ...

int main() {
    // Directory check - MANDATORY for method tests
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            return 1;
        }
    }

    // Clean state - prevent pollution from previous runs
    ar_system__shutdown(own_system);
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agerun.agency");

    // Test setup
    ar_methodology__register_method("test", "1.0.0", "../methods/test-1.0.0.method");
    
    // ... test code ...
    
    // Process all messages before cleanup
    while (ar_system__process_next_message(own_system));
    
    // Initialize system after creating methods
    ar_system__init(own_system, NULL, NULL);
    
    return 0;
}
```

## Generalization
1. **Directory verification**: Tests must run from bin/ directory
2. **State cleanup**: Reset global state before tests
3. **File cleanup**: Remove persistence files
4. **Message processing**: Process all messages to prevent leaks
5. **Late initialization**: Init system after method registration

## Implementation
Use this template for all method tests:
1. Copy the template structure
2. Add specific test logic after "test code" comment
3. Ensure all messages are processed before cleanup
4. Run with `make test_name` from repo root

Common issues prevented:
- Wrong directory errors
- State pollution between test runs
- Memory leaks from unprocessed messages
- Persistence file conflicts

## Related Patterns
- [BDD Test Structure](bdd-test-structure.md)
- [Test Function Naming Accuracy](test-function-naming-accuracy.md)
- [Method Test AST Verification](method-test-ast-verification.md)
- [AgeRun Method Language Nesting Constraint](agerun-method-language-nesting-constraint.md)