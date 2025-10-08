# Redundant Test Cleanup Anti-Pattern

## Learning
Tests often perform cleanup that's already handled by test fixtures, creating redundant code that can mask fixture bugs and slow down tests. Always verify what fixtures actually do before adding defensive cleanup.

## Importance
Redundant cleanup code:
- Makes tests harder to maintain
- Can hide fixture bugs by working around them
- Slows down test execution
- Creates confusion about responsibility

## Example
Found in multiple test files during ar_agency global API removal:
```c
// ANTI-PATTERN: Test doing cleanup already handled by fixture
void test_spawn_instruction__creates_agent() {
    InterpreterFixture* fixture = create_fixture();

    // ... test logic ...

    // REDUNDANT - fixture destroy already does this:
    ar_agency__reset(fixture->agency);  // Unnecessary!

    destroy_fixture(fixture);
}

// BETTER: Trust the fixture
void test_spawn_instruction__creates_agent() {
    InterpreterFixture* fixture = create_fixture();

    // ... test logic ...

    destroy_fixture(fixture);  // Fixture handles all cleanup
}

// Fixture already handles cleanup:
void destroy_fixture(InterpreterFixture* own_fixture) {
    if (own_fixture) {
        // Destroy system (which destroys agency internally)
        ar_system__destroy(own_fixture->own_system);

        // The above already resets agency - no need for explicit reset!
        ar_heap__free(own_fixture);
    }
}
```

## Generalization
1. Before adding cleanup code, check what the fixture destroy does
2. Trace through the complete destruction chain
3. Remove redundant cleanup calls
4. Document fixture responsibilities clearly
5. If cleanup seems necessary, fix the fixture instead

## Implementation
```bash
# Find potentially redundant cleanup in tests
grep -n "ar_agency__reset\|ar_methodology__cleanup" *_tests.c | \
  while read line; do
    file=$(echo $line | cut -d: -f1)
    # Check if file uses fixtures
    if grep -q "create_fixture\|destroy_fixture" "$file"; then
      echo "Potential redundancy in: $line"
    fi
  done
```

## Related Patterns
- [Test Isolation Shared Directory Pattern](test-isolation-shared-directory-pattern.md)
- [Test Fixture Module Creation Pattern](test-fixture-module-creation-pattern.md)