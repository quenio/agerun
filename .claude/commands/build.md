Execute comprehensive build verification with minimal output and check for hidden issues.
```bash
make build 2>&1 && make check-logs
```

Note: Using `2>&1` to capture stderr ensures all warnings and errors are visible. The `check-logs` target verifies no hidden issues in build log files.

**Important**: The build system runs parallel tests with different compilers (gcc for regular tests, clang for Thread Sanitizer). Each uses isolated build directories to prevent conflicts ([details](../../kb/compiler-output-conflict-pattern.md)).

**Exit Code Handling**: When using popen() to run processes, remember that exit codes are multiplied by 256 ([details](../../kb/exit-code-propagation-popen.md)). Non-critical operations should follow graceful degradation patterns ([details](../../kb/graceful-degradation-pattern.md)).