# Test-First Verification Practice

## Learning
When fixing a specific test failure, run that single test first to verify the fix works before running the full build suite. This provides faster feedback and saves time by avoiding unnecessary full builds when the fix isn't complete.

## Importance
Full builds can take 45-60 seconds or more. Running a single test takes 1-2 seconds. Getting immediate feedback on whether a fix works allows rapid iteration. If the single test passes, then run the full suite to ensure no regressions.

## Example
```bash
# Workflow when fixing ar_system_tests failure

# 1. Make the fix in ar_system.c
vim modules/ar_system.c

# 2. Run ONLY the failing test first (1-2 seconds)
make ar_system_tests 2>&1

# 3. If test passes, THEN run full build (45-60 seconds)
make clean build 2>&1

# AVOID: Making fix and immediately running full build
# This wastes time if the fix is incomplete
```

## Generalization
Test-first verification workflow:
1. **Identify failing test**: Note which specific test needs fixing
2. **Make targeted fix**: Edit only necessary code
3. **Run single test**: Verify fix works in isolation
4. **Iterate if needed**: Refine fix based on test output
5. **Run full suite**: Only after single test passes
6. **Check for regressions**: Ensure fix didn't break other tests

## Implementation
```bash
#!/bin/bash
# test_fix.sh - Helper for test-first verification

TEST_NAME=$1
if [ -z "$TEST_NAME" ]; then
    echo "Usage: $0 <test_name>"
    exit 1
fi

echo "=== Running single test: $TEST_NAME ==="
if make "$TEST_NAME" 2>&1; then
    echo "✓ Single test passed!"
    echo ""
    echo "=== Running full test suite ==="
    make clean build 2>&1
else
    echo "✗ Single test failed - fix and retry"
    exit 1
fi
```

### Common Patterns
```bash
# Memory leak fix verification
make ar_agent_tests 2>&1 | grep "Actual memory leaks"

# Warning elimination verification  
make ar_system_tests 2>&1 | grep -i "warning"

# Specific test function verification
make ar_interpreter_tests 2>&1 | grep "test_method_evaluation"

# Quick error check
make ar_method_tests 2>&1 | tail -20
```

## Related Patterns
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [Test Build Before Run Pattern](test-build-before-run-pattern.md)
- [Make-Only Test Execution](make-only-test-execution.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)