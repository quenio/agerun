# Feature Remnant Cleanup Pattern

## Learning
When removing a feature from a codebase, the functional code is only part of what needs cleanup. Comments, debug output, test infrastructure, and documentation all contain remnants that must be systematically removed to prevent confusion and technical debt accumulation.

## Importance
Feature remnants create confusion for future developers who encounter references to non-existent functionality. They waste time trying to understand obsolete patterns, may attempt to "fix" broken references, and perpetuate outdated coding patterns through copy-paste. Complete cleanup ensures the codebase accurately reflects its current architecture.

## Example
```c
// Found during wake/sleep removal - remnants across 19 files:

// 1. Obsolete comments
// ar_agent.c:60
// No longer send wake message on creation - agents initialize themselves

// 2. Debug output referencing removed feature  
// grade_evaluator_tests.c:52
printf("DEBUG: About to process wake message for agent %lld\n", (long long)evaluator_agent);

// 3. Test infrastructure for non-existent behavior
// calculator_tests.c:50
// Process wake message
ar_method_fixture__process_next_message(own_fixture);

// 4. Assertions about removed functionality
// ar_agent_tests.c:48
assert(!message_processed && "No wake message should be sent on agent creation");

// 5. Commented code waiting for removed feature
// bootstrap_tests.c:59 (in 77-line commented block)
// Process wake message (bootstrap should spawn echo and send context)
bool processed = ar_method_fixture__process_next_message(own_fixture);
AR_ASSERT(processed, "Wake message should be processed");
```

## Generalization
Feature removal requires systematic cleanup across multiple artifact types:
1. **Functional code** - The actual implementation
2. **Comments** - Explanations referencing the feature
3. **Debug output** - Print statements mentioning the feature
4. **Test code** - Infrastructure expecting the feature's behavior
5. **Documentation** - User guides, API docs, inline comments
6. **Error handling** - Whitelists, special cases for the feature
7. **Build configuration** - Flags, defines related to the feature

## Implementation
```bash
# Comprehensive remnant removal process

# 1. Identify all references to removed feature
grep -r "feature_name" --include="*.c" --include="*.h" --include="*.md" .

# 2. Categorize findings
# - Functional code (must remove)
# - Comments (update or remove)
# - Test code (remove if testing removed behavior)
# - Documentation (update to reflect current state)

# 3. Clean systematically by category
# Start with tests to ensure they still pass after each change

# 4. Verify complete removal
grep -r "feature_name" . | grep -v ".git"
# Should return no results

# 5. Run full test suite
make clean build 2>&1
make check-logs

# 6. Review diff before committing
git diff --stat
# Ensure changes are consistent and complete
```

## Related Patterns
- [Regression Test Removal Criteria](regression-test-removal-criteria.md)
- [Systematic Parameter Removal Refactoring](systematic-parameter-removal-refactoring.md)
- [Non-Functional Code Detection Pattern](non-functional-code-detection-pattern.md)
- [Phased Cleanup Pattern](phased-cleanup-pattern.md)
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)