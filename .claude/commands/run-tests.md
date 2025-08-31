Build and run all tests.


# Run Tests
## Command
```bash
make run-tests 2>&1
```

## Expected Output

### Success State
```
Building library...
Running all tests...
  ✓ ar_agent_tests
  ✓ ar_agency_tests
  ✓ ar_data_tests
  ✓ ar_expression_parser_tests
  ✓ ar_instruction_parser_tests
  ✓ ar_method_evaluator_tests
  ✓ ar_string_tests
  ... (61 more tests)
All 68 tests passed! (1126 assertions)
```

### Failure States

**Test Failure:**
```
Building library...
Running all tests...
  ✓ ar_agent_tests
  ✗ ar_data_tests
    FAILED at ar_data_tests.c:45
    Expected: 42
    Actual: 0
  ✓ ar_expression_parser_tests
  ... (65 more tests)
67 of 68 tests passed
make: *** [run-tests] Error 1
```

**Memory Leak:**
```
Building library...
Running all tests...
All 68 tests passed! (1126 assertions)

WARNING: 3 memory leaks detected!
See bin/run-tests/memory_report_*.log for details
```

**Compilation Error:**
```
Building library...
modules/ar_data.c:123: error: expected ';' before '}' token
make: *** [ar_data.o] Error 1
```

## Key Points

- **Always use `2>&1`** to capture memory leak warnings
- **Individual test reports** at `bin/run-tests/memory_report_<test>.log`
- **Zero tolerance for leaks** - any leak should be fixed immediately
- Tests run in isolated processes for safety

**Test Infrastructure**: Tests requiring shared setup/teardown should use fixture modules with opaque types and proper lifecycle management ([details](../../kb/test-fixture-module-creation-pattern.md)). Fixtures should evolve organically by adding helper methods as needs arise ([details](../../kb/test-fixture-evolution-pattern.md)).

**Test Effectiveness**: When testing error paths, temporarily break implementation to verify tests detect failures ([details](../../kb/test-effectiveness-verification.md)). Use filesystem permissions (chmod) to create predictable failure conditions ([details](../../kb/permission-based-failure-testing.md)). For untestable conditions, use dlsym interception ([details](../../kb/dlsym-test-interception-technique.md)).

**Test Quality**: Write strong tests that verify specific outcomes, not just success/failure ([details](../../kb/test-assertion-strength-patterns.md)). Always use make to run tests, never execute binaries directly ([details](../../kb/make-only-test-execution.md)).