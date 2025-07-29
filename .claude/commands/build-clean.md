Run a clean build for comprehensive build verification with minimal output.

```bash
make clean build 2>&1
```

Note: Using `2>&1` to capture stderr ensures all warnings and errors are visible.