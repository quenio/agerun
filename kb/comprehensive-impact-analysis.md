# Comprehensive Impact Analysis

## Learning
After making structural changes to core functionality, systematically analyze impact across all code layers: main implementation → unit tests → test fixtures → integration tests → entire codebase. This ensures no affected code is missed and prevents inconsistencies.

## Importance
- Catches all code affected by structural changes
- Prevents inconsistent state across codebase
- Reveals unexpected dependencies
- Ensures comprehensive cleanup
- Maintains codebase coherence

## Example
```c
// Scenario: After removing wake messages from agent creation

// Layer 1: Main Implementation
// Check: ar_agent.c, ar_system.c
git diff HEAD~1 modules/ar_agent.c modules/ar_system.c

// Layer 2: Unit Tests  
// Check: ar_agent_tests.c, ar_system_tests.c
grep -n "Process.*wake" modules/ar_system_tests.c
// Found: 4 outdated comments about processing wake messages

// Layer 3: Test Fixtures
// Check: All fixture modules
grep -n "wake" modules/*fixture*.c
// Found: ar_interpreter_fixture.c has 2 wake processing calls

// Layer 4: All Test Files
grep -l "Process wake" modules/*_tests.c
// Found: 8 test files with outdated wake processing

// Layer 5: Method Tests
grep -l "Process wake" methods/*_tests.c  
// Found: 8 method test files need cleanup

// Layer 6: Entire Codebase
grep -r "wake" . --include="*.c" --include="*.h" | wc -l
// Found: 453 total references (many deferred to future cycles)

// Result: Systematic cleanup of 24+ outdated references
```

## Generalization
Impact analysis layers (check in order):
1. **Core Implementation**: Files directly modified
2. **Direct Tests**: Unit tests for modified components
3. **Test Infrastructure**: Fixtures, helpers, utilities
4. **Integration Layer**: Tests that use modified components
5. **Related Subsystems**: Components that interact with changes
6. **Documentation**: .md files, comments, examples
7. **Entire Codebase**: Comprehensive search for any references

Search strategies per layer:
- Use targeted grep for specific patterns
- Check git diff for direct changes
- Search for function names that changed
- Look for related concept keywords
- Review import/include statements

## Implementation
```bash
#!/bin/bash
# Comprehensive impact analysis script

SEARCH_TERM="$1"  # e.g., "wake", "process_next_message"

echo "=== Layer 1: Direct Changes ==="
git diff HEAD~1 --name-only | xargs grep -l "$SEARCH_TERM"

echo "=== Layer 2: Unit Tests ==="
grep -l "$SEARCH_TERM" modules/*_tests.c

echo "=== Layer 3: Test Fixtures ==="
grep -l "$SEARCH_TERM" modules/*fixture*.c

echo "=== Layer 4: Method Tests ==="
grep -l "$SEARCH_TERM" methods/*_tests.c

echo "=== Layer 5: Documentation ==="
grep -l "$SEARCH_TERM" modules/*.md methods/*.md

echo "=== Layer 6: Full Codebase ==="
echo "Total references: $(grep -r "$SEARCH_TERM" . --include="*.c" --include="*.h" | wc -l)"

echo "=== Summary ==="
echo "Files needing attention:"
grep -rl "$SEARCH_TERM" . --include="*.c" --include="*.h" | head -20
```

## Related Patterns
- [Phased Cleanup Pattern](phased-cleanup-pattern.md)
- [Fixture Dependency Mapping](fixture-dependency-mapping.md)
- [Code Movement Verification](code-movement-verification.md)