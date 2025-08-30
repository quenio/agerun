Run address sanitizer on all tests for detecting memory issues.
```bash
make sanitize-tests 2>&1
```

Note: Using `2>&1` to capture stderr ensures all sanitizer output and warnings are visible.

**Important**: Tests using dlsym for function interception (named `*_dlsym_tests`) are automatically excluded from sanitizer builds to prevent conflicts. These tests still run in normal builds. ([details](../../kb/sanitizer-test-exclusion-pattern.md), [technique](../../kb/dlsym-test-interception-technique.md))

**Test Validation**: When testing error handling, use permission-based failure testing with chmod to create predictable failures ([details](../../kb/permission-based-failure-testing.md)). Always verify test effectiveness by temporarily breaking the implementation ([details](../../kb/test-effectiveness-verification.md)).