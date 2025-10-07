# Standards Over Expediency Principle

## Learning
When faced with a choice between following project standards and taking shortcuts for expediency, always choose standards. Even when existing code doesn't comply with standards, new code must comply. The rationale for choosing standards should be deducible from project documentation and established patterns.

## Importance
Expedient shortcuts create technical debt that compounds over time. Standards exist to maintain code quality, consistency, and maintainability. When developers take shortcuts "just this once," it:
- Sets precedent for future violations
- Makes the codebase inconsistent
- Reduces diagnostic capability
- Signals that standards are optional

Users expect developers to reason through standards choices independently, not ask for permission to violate them.

## Example
```c
// Scenario: Need to add assertions to test
// Existing file uses assert(), but project standard is AR_ASSERT

// BAD: Expedient approach
void test_agent_store_memory_restoration(void) {
    ar_data_t *mut_memory = ar_agent_store__get_agent_memory(store, 10);

    // "File uses assert(), so I'll use assert() too"
    assert(mut_memory != NULL);  // No diagnostic message
    assert(strcmp(ar_data__get_map_string(mut_memory, "name"), "test") == 0);

    // User challenges: "Why replaced asserts?"
}

// GOOD: Standards-first approach
#include "ar_assert.h"  // Add required header

void test_agent_store_memory_restoration(void) {
    ar_data_t *mut_memory = ar_agent_store__get_agent_memory(store, 10);

    // Use AR_ASSERT despite existing code using assert()
    AR_ASSERT(mut_memory != NULL, "Agent should have memory after load");
    const char *name = ar_data__get_map_string(mut_memory, "name");
    AR_ASSERT(strcmp(name, "test") == 0, "Memory value should be preserved");

    // Rationale: AGENTS.md states "AR_ASSERT macros" are required
}
```

## Generalization
**Decision framework when facing standards vs expediency**:

1. **Check project documentation**:
   ```bash
   # Look for explicit requirements
   grep -i "required\|must\|mandatory" AGENTS.md README.md
   ```

2. **Look for KB articles on the topic**:
   ```bash
   grep -l "assertion\|test.*standard" kb/*.md
   ```

3. **Examine the rationale**:
   - Standards usually have good reasons (diagnostics, consistency, safety)
   - Expedient shortcuts usually have costs (tech debt, confusion)

4. **Choose standards and articulate why**:
   - "Project requires AR_ASSERT per AGENTS.md line 157"
   - "Existing code needs updating, not replicating"
   - "Better diagnostics worth the extra effort"

5. **Never ask for permission to violate**:
   - User expects independent reasoning
   - Asking signals lack of conviction in standards

## Implementation
```c
// Pattern: When encountering standard vs expedient choice

// Step 1: Identify the conflict
// EXAMPLE: Existing code vs documented standard
// existing_code_uses = assert()
// documented_standard = AR_ASSERT()

// Step 2: Search for documentation
#include "ar_assert.h"  // Required by AGENTS.md
// grep "AR_ASSERT" AGENTS.md → "Test Requirements: AR_ASSERT macros"

// Step 3: Choose standard with rationale
AR_ASSERT(condition, "Descriptive message");  // Following AGENTS.md requirement

// Step 4: Defend choice when challenged
// "AGENTS.md line 157 requires AR_ASSERT macros for tests.
//  Better diagnostics justify the extra header include.
//  Existing file needs updating to comply with standards."
```

## Common Expedient Shortcuts to Avoid
1. **"Match existing code style"** - Existing code may be wrong
2. **"It's faster this way"** - Technical debt costs more later
3. **"Just for this one case"** - Sets bad precedent
4. **"The test still works"** - Works ≠ maintainable
5. **"I'll fix it later"** - Later rarely comes

## Evidence-Based Decision Making
To determine what standards apply:

```bash
# 1. Check AGENTS.md guidelines
grep -A 5 "Test Requirements" AGENTS.md

# 2. Look for KB articles
grep -l "assertion\|AR_ASSERT" kb/*.md

# 3. Examine similar code for patterns
grep -r "AR_ASSERT" modules/*_tests.c | head -5

# 4. Verify with documentation checker
make check-docs
```

## User Expectations
When user asks "Which one do you believe is my preference?", they expect:
- Independent reasoning from available evidence
- Articulation of standards-based rationale
- Confidence in choosing correct approach
- Learning from the challenge to improve future decisions

NOT:
- Asking for shortcuts
- Defending expediency
- Waiting for explicit permission

## Quality Indicators
Signs of proper standards application:
- New code exceeds quality of code it modifies
- Documentation references provided for choices
- Rationale articulated without being asked
- Standards applied consistently across all changes

## Related Patterns
- [Const-Correctness Principle](const-correctness-principle.md)
- [No Parallel Implementations Principle](no-parallel-implementations-principle.md)
- [BDD Test Structure](bdd-test-structure.md)
- [AR Assert Descriptive Failures](ar-assert-descriptive-failures.md)
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)
