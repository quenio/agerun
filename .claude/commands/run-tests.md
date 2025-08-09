Build and run all tests.

```bash
make run-tests 2>&1
```

Note: Using `2>&1` to capture stderr ensures memory leak warnings and other diagnostic messages are visible.

**Test Infrastructure**: Tests requiring shared setup/teardown should use fixture modules with opaque types and proper lifecycle management ([details](../../kb/test-fixture-module-creation-pattern.md)).