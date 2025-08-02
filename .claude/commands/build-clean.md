Run a clean build for comprehensive build verification with minimal output and check for hidden issues.

```bash
make clean build 2>&1 && make check-logs
```

Note: Using `2>&1` to capture stderr ensures all warnings and errors are visible. The `check-logs` target verifies no hidden issues in build log files.