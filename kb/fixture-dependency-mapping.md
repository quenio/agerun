# Fixture Dependency Mapping

## Learning
When planning changes to test fixtures, it's critical to map which tests depend on each fixture to ensure all affected tests are updated together and to place fixture updates within the appropriate TDD cycles.

## Importance
Test fixtures are shared infrastructure. Changing a fixture without updating all dependent tests causes cascading failures. Understanding dependencies prevents broken builds and ensures changes are properly scoped.

## Example
```c
// Mapping fixture dependencies before planning changes
// ar_system_fixture is used by:
// - ar_agent_tests.c
// - ar_agent_update_tests.c  
// - ar_agency_tests.c

// ar_method_fixture is used by:
// - bootstrap_tests.c
// - echo_tests.c
// - calculator_tests.c
// - (and 5 other method tests)

// Planning changes with dependencies in mind:
// TDD Cycle 1: Update ar_agent
//   Iteration 1.1: Change ar_agent behavior
//   Iteration 1.2: Update ar_system_fixture (affects 3 test files)
//   Must verify all 3 dependent tests pass

// TDD Cycle 4: Update all methods
//   Iteration 4.1: Update bootstrap method
//   Iteration 4.2: Update ar_method_fixture (affects ALL method tests)
//   Must update all 8 method tests in this cycle
```

## Generalization
Before modifying fixtures:
1. Identify all tests that use the fixture
2. Plan to update fixture and all dependent tests together
3. Place fixture updates in the cycle that modifies dependent modules
4. Never update a fixture without updating its dependents
5. Consider if fixture changes reveal design coupling issues

## Implementation
```bash
# Find all tests using a specific fixture
grep -l "ar_system_fixture" modules/*_tests.c methods/*_tests.c

# Find all fixture functions to understand interface
grep "ar_system_fixture__" modules/ar_system_fixture.h

# Map dependencies before planning
echo "Fixture: ar_system_fixture"
echo "Used by:"
grep -l "ar_system_fixture__" **/*_tests.c | sed 's/^/  - /'

# Document in TDD plan
# "Iteration X.Y: Update ar_system_fixture (used by A, B, C tests)"
```

## Related Patterns
- [Test Fixture Module Creation Pattern](test-fixture-module-creation-pattern.md)
- [Dynamic Test Resource Allocation](dynamic-test-resource-allocation.md)
- [Multi-Session TDD Planning](multi-session-tdd-planning.md)
- [Test Standardization Retroactive](test-standardization-retroactive.md)