Run build for comprehensive build verification with minimal output and check for hidden issues.

```bash
make build 2>&1 && make check-logs
```

Note: Using `2>&1` to capture stderr ensures all warnings and errors are visible. The `check-logs` target verifies no hidden issues in build log files.

**Important**: The build system runs parallel tests with different compilers (gcc for regular tests, clang for Thread Sanitizer). Each uses isolated build directories to prevent conflicts ([details](../../kb/compiler-output-conflict-pattern.md)).