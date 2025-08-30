Build and run all tests.
```bash
make run-tests 2>&1
```

Note: Using `2>&1` to capture stderr ensures memory leak warnings and other diagnostic messages are visible.

**Test Infrastructure**: Tests requiring shared setup/teardown should use fixture modules with opaque types and proper lifecycle management ([details](../../kb/test-fixture-module-creation-pattern.md)). Fixtures should evolve organically by adding helper methods as needs arise ([details](../../kb/test-fixture-evolution-pattern.md)).

**Test Effectiveness**: When testing error paths, temporarily break implementation to verify tests detect failures ([details](../../kb/test-effectiveness-verification.md)). Use filesystem permissions (chmod) to create predictable failure conditions ([details](../../kb/permission-based-failure-testing.md)). For untestable conditions, use dlsym interception ([details](../../kb/dlsym-test-interception-technique.md)).

**Test Quality**: Write strong tests that verify specific outcomes, not just success/failure ([details](../../kb/test-assertion-strength-patterns.md)). Always use make to run tests, never execute binaries directly ([details](../../kb/make-only-test-execution.md)).