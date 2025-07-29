Run static analysis on the executable code.

```bash
make analyze-exec 2>&1
```

Note: Using `2>&1` to capture stderr ensures all analyzer warnings are visible.